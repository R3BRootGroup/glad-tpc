/******************************************************************************
 *   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2018-2025 Members of R3B Collaboration                     *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU Lesser General Public Licence (LGPL) version 3,        *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TClonesArray.h"
#include "TMath.h"

#include "R3BGTPC.h"
#include "R3BGTPCCal2Hit.h"
#include "R3BGladFieldMap.h"

R3BGTPCCal2Hit::R3BGTPCCal2Hit()
    : PadCoordArr(boost::extents[5632][4][2])
    , FairTask("R3B GTPC Cal to Hit")
    , fCalCA(NULL)
    , fHitCA(NULL)
    , fTPCMap(NULL)
    , fOnline(kFALSE)
    , fLangevinBack(kTRUE)
{
    fTPCMap = std::make_shared<R3BGTPCMap>();
}

R3BGTPCCal2Hit::~R3BGTPCCal2Hit()
{
    LOG(info) << "R3BGTPCCal2Hit: Delete instance";
    if (fHitCA)
        delete fHitCA;
    if (fCalCA)
        delete fCalCA;
}

void R3BGTPCCal2Hit::SetParContainers()
{
    // Reading Parameter Containers
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    if (!rtdb)
    {
        LOG(error) << "R3BGTPCCal2Hit:: FairRuntimeDb not opened!";
    }

    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(fatal) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCGeoPar";
        return;
    }

    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(fatal) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(fatal) << "R3BGTPCCal2Hit::SetParContainers: No R3BGTPCElecPar";
        return;
    }
}

void R3BGTPCCal2Hit::SetParameter()
{
    // From gas properties
    fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]
    fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]
    fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]
    fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]
    fFanoFactor = fGTPCGasPar->GetFanoFactor();
    // From geo properties
    fHalfSizeTPC_X = fGTPCGeoPar->GetActiveRegionx() / 2.; //[cm]
    fHalfSizeTPC_Y = fGTPCGeoPar->GetActiveRegiony() / 2.; //[cm]
    fHalfSizeTPC_Z = fGTPCGeoPar->GetActiveRegionz() / 2.; //[cm]
    fSizeOfVirtualPad = fGTPCGeoPar->GetPadSize();         // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    fOffsetX = fGTPCGeoPar->GetTargetOffsetX();            // X offset [cm]
    fOffsetZ = fGTPCGeoPar->GetTargetOffsetZ();            // Z offset [cm]
    fTargetOffsetX = fGTPCGeoPar->GetGladOffsetX();
    fTargetOffsetY = fGTPCGeoPar->GetGladOffsetY();
    fTargetOffsetZ = fGTPCGeoPar->GetGladOffsetZ();
    // From electronic properties
    fDriftEField = fGTPCElecPar->GetDriftEField();     // [V/cm]
    fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // [ns]
    fTimeBinSize = fGTPCElecPar->GetTimeBinSize();     // [ns]
}

InitStatus R3BGTPCCal2Hit::Init()
{
    LOG(info) << "R3BGTPCCal2Hit::Init() ";
    assert(!fHitCA); // in case someone calls Init() twice.

    // INPUT DATA - Cal
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
    {
        LOG(fatal) << "Init: No FairRootManager";
    }

    fCalCA = (TClonesArray*)ioManager->GetObject("GTPCCalData");
    if (!fCalCA)
    {
        LOG(fatal) << "Init: No R3BGTPCCalData";
    }

    // Register output - Hit
    fHitCA = new TClonesArray("R3BGTPCHitData", 50);

    if (!fOnline)
    {
        ioManager->Register("GTPCHitData", "GTPC Hit", fHitCA, kTRUE);
    }
    else
    {
        ioManager->Register("GTPCHitData", "GTPC Hit", fHitCA, kFALSE);
    }

    SetParameter();

    fTPCMap->GeneratePadPlane();

    return kSUCCESS;
}

InitStatus R3BGTPCCal2Hit::ReInit()
{
    SetParContainers();
    SetParameter();
    return kSUCCESS;
}

void R3BGTPCCal2Hit::Exec(Option_t* opt)
{
    Reset(); // Reset entries in output arrays, local arrays

    // ALGORITHMS FOR HIT FINDING
    // Nb of CrystalHits in current event
    Int_t nCals = fCalCA->GetEntries();
    LOG(info) << "R3BGTPCCal2Hit: processing " << nCals << " CalPads";

    if (!nCals)
    {
        LOG(warn) << "No CalPads";
    }

    Double_t x = 0, y = 0, z = 0, lW = 0, ene = 0;
    Double_t xold = 0, yold = 0, zold = 0;
    Double_t xnew = 0, ynew = 0, znew = 0;

    R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField(); // B Field
    if (!gladField)
    {
        LOG(warn) << "No GladField";
    }

    R3BGTPCCalData** calData;
    calData = new R3BGTPCCalData*[nCals];

    // from create_tpc_geo_test.C (geo in file
    // R3BRoot/glad-tpc/geometry/gladTPC_test.geo.root)
    Double_t TargetOffsetX = fTargetOffsetX;
    Double_t TargetOffsetY = fTargetOffsetY;
    Double_t TargetOffsetZ = fTargetOffsetZ; // USING INSTEAD THE FIELD MAP DESPLACEMENT! MISMATCH
    Double_t TargetOffsetZ_FM = fTargetOffsetZ;

    // Those values seem not to be anywhere
    Double_t TargetAngle = 14. * TMath::Pi() / 180;
    fOffsetX = 0;
    fOffsetZ = 0;

    for (Int_t i = 0; i < nCals; i++)
    {
        calData[i] = (R3BGTPCCalData*)(fCalCA->At(i));
        UShort_t pad = calData[i]->GetPadId();
        std::vector<UShort_t> adc_cal = calData[i]->GetADC();

        Double_t counts = 0;
        Double_t time = 0;
        Double_t cloudLong = 0; // step by step
        Double_t cloudTransv = 0;
        Double_t sigmaLong = 0; // aprox for the whole time of reconstruction
        Double_t sigmaTransv = 0;

        // To store all the hit weighted mean variables
        Double_t pad_counts = 0;
        Double_t hitx = 0;
        Double_t hity = 0;
        Double_t hitz = 0;
        Double_t hitlW = 0;

        for (auto iadc = 0; iadc < adc_cal.size(); iadc++)
        {
            counts = adc_cal[iadc];
            time = iadc; //[timeBuckets]

            // Important to take only non zero values
            if (counts == 0)
            {
                continue;
            }

            auto PadCenterCoord = fTPCMap->CalcPadCenter(pad);
            // Invalid ID condition PadCenterCoord[0]=-9999 (Should be solved in
            // R3BGTPCLangevin)
            if (PadCenterCoord[0] < -9000)
            {
                LOG(warn) << "R3BGTPCCal2Hit::Exec Invalid padID";
                continue;
            }

            z = PadCenterCoord[0] / 10.0; //[cm] (PadCenterCoord on mm)
            x = PadCenterCoord[1] / 10.0;
            y = -fHalfSizeTPC_Y; // Start at pad plane

            x = x + fOffsetX; //[cm]
            z = z + fOffsetZ; //[cm]

            xold = x;
            yold = y;
            zold = z;

            // Transformation from tcp coordinates to glad coordinates
            x = cos(-TargetAngle) * (xold) + sin(-TargetAngle) * (zold);
            z = (TargetOffsetZ_FM - fHalfSizeTPC_Z) - sin(-TargetAngle) * (xold) + cos(-TargetAngle) * (zold);

            time = time * fTimeBinSize + 0.5 * fTimeBinSize; //[ns] moving from TimeBuckets to ns; adding the
                                                             // half of
                                                             // the size of the bin to take the center of the bin

            // Reconstruction without Langevin
            if (fLangevinBack == kFALSE)
            {
                y = y + time * fDriftVelocity; // [cm] Simple projection case -> Same
                                               // x,z just moving in coord y
            }
            // Reconstruction with Langevin
            if (fLangevinBack == kTRUE)
            {
                Double_t accDriftTime = time; // Making a copy for loop discounting
                Double_t E_x = 0;
                Double_t E_z = 0;

                Double_t E_y = fDriftEField; // [V/cm]

                Double_t B_x = 0;
                Double_t B_y = 0;
                Double_t B_z = 0;
                Double_t moduleB = 0;

                Double_t vDrift_x = 0;
                Double_t vDrift_y = 0;
                Double_t vDrift_z = 0;

                Double_t cteMult = 0;
                Double_t cteMod = 0;
                Double_t productEB = 0;

                Double_t recoverDriftTimeStep = 0.;
                Bool_t recoverStep = kTRUE;

                Double_t mu = fDriftVelocity / E_y; // [cm^2 ns^-1 V^-1]

                // Auxiliar values to obtain the velocities after the first callback to
                // make the second and definitive callback
                Double_t auxx;
                Double_t auxy;
                Double_t auxz;

                sigmaLong = sqrt(time * 2 * fLongDiff);
                sigmaTransv = sqrt(time * 2 * fTransDiff);
                cloudLong = 0;
                cloudTransv = 0;

                LOG(debug) << "R3BGTPCCal2Hit::Exec, INITIAL VALUES: \tTimeToRun=" << accDriftTime << " [ns]"
                           << " \tx=" << x << "  \ty=" << y << " \tz=" << z << " [cm]";

                // Calculation Loop till accDriftTime = 0
                while (accDriftTime > 0.)
                {
                    // We adjust the time for the last step before reaching time=0
                    if (accDriftTime - fDriftTimeStep < 0.0)
                    {
                        if (recoverStep)
                        {
                            recoverDriftTimeStep = fDriftTimeStep;
                        }
                        recoverStep = kFALSE;
                        fDriftTimeStep = accDriftTime;
                    }

                    B_x = 1e4 * gladField->GetBx(x, y,
                                                 z); // Field components return in [kG], moved to [V ns cm^-2]
                    B_y = 1e4 * gladField->GetBy(x, y, z);
                    B_z = 1e4 * gladField->GetBz(x, y, z);

                    moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // [V ns cm^-2]
                    cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // dimensionless
                    cteMult = mu * cteMod;                                    // [cm^2 V^-1 ns^-1]
                    productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; [V^2 ns cm^-3]

                    // Drift velocities for auxiliar point finding
                    vDrift_x = cteMult * (mu * (E_y * B_z) + mu * mu * productEB * B_x);  //[cm/ns]
                    vDrift_y = cteMult * (E_y + mu * mu * productEB * B_y);               //[cm/ns]
                    vDrift_z = cteMult * (mu * (-E_y * B_x) + mu * mu * productEB * B_z); //[cm/ns]

                    // Point where we calculate the velocity vector for reversion
                    auxx = x - vDrift_x * fDriftTimeStep;
                    auxy = y + vDrift_y * fDriftTimeStep;
                    auxz = z - vDrift_z * fDriftTimeStep;

                    // Field in the auxiliar point
                    B_x = 1e4 * gladField->GetBx(auxx, auxy, auxz);
                    B_y = 1e4 * gladField->GetBy(auxx, auxy, auxz);
                    B_z = 1e4 * gladField->GetBz(auxx, auxy, auxz);

                    moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // [V ns cm^-2]
                    cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // dimensionless
                    cteMult = mu * cteMod;                                    // [cm^2 V^-1 ns^-1]
                    productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; [V^2 ns cm^-3]

                    // Drift velocities
                    vDrift_x = cteMult * (mu * (E_y * B_z) + mu * mu * productEB * B_x);  //[cm/ns]
                    vDrift_y = cteMult * (E_y + mu * mu * productEB * B_y);               //[cm/ns]
                    vDrift_z = cteMult * (mu * (-E_y * B_x) + mu * mu * productEB * B_z); //[cm/ns]

                    // Use vector velocity (reversed) in the initial point to move
                    // backwards
                    x = x - vDrift_x * fDriftTimeStep;
                    y = y + vDrift_y * fDriftTimeStep;
                    z = z - vDrift_z * fDriftTimeStep;

                    // Taking account of clouds widths
                    cloudLong += fDriftTimeStep * 2 * fLongDiff;
                    cloudTransv += fDriftTimeStep * 2 * fTransDiff * cteMod;

                    // Resting time update
                    accDriftTime = accDriftTime - fDriftTimeStep;
                    LOG(debug) << "R3BGTPCCal2Hit::Exec, NEW VALUES: accDriftTime=" << accDriftTime << " [ns]"
                               << " x=" << x << " y=" << y << " z=" << z << " [cm]"
                               << " Drift_v " << vDrift_x << " fDriftTimeStep " << fDriftTimeStep;
                }
                // Once the last step adjust is done, reset the variables.
                if (recoverDriftTimeStep)
                {
                    fDriftTimeStep = recoverDriftTimeStep;
                    recoverDriftTimeStep = 0.;
                    recoverStep = kTRUE;
                }
                // Comparing sigmas obtained in both ways
                LOG(debug) << "Comparing sigmas... Approx: " << sigmaLong << " " << sigmaTransv
                           << ";  Step by step: " << TMath::Sqrt(cloudLong) << " " << TMath::Sqrt(cloudTransv);
            }
            // Adding the hit relevant info for the mean

            xnew = x;
            ynew = y;
            znew = z;

            // Back to tpc coordinates
            x = +cos(TargetAngle) * xnew + sin(TargetAngle) * (znew - (TargetOffsetZ_FM - fHalfSizeTPC_Z));
            z = -sin(TargetAngle) * xnew + cos(TargetAngle) * (znew - (TargetOffsetZ_FM - fHalfSizeTPC_Z));

            hitx += x * counts;
            hity += y * counts;
            hitz += z * counts;
            hitlW += sigmaLong * counts;
            pad_counts += counts;
        }
        // Final Hit values calculated by weighted mean
        hitx = hitx / pad_counts;
        hity = hity / pad_counts;
        hitz = hitz / pad_counts;
        hitlW = hitlW / pad_counts;
        AddHitData(hitx, hity, hitz, hitlW, pad_counts);
    }

    if (calData)
        delete calData;
    return;
}

void R3BGTPCCal2Hit::Finish() {}

void R3BGTPCCal2Hit::Reset()
{
    LOG(debug) << "Clearing HitData Structure";
    if (fHitCA)
        fHitCA->Clear();
}

R3BGTPCHitData* R3BGTPCCal2Hit::AddHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy)
{
    // It fills the R3BGTPCCalData
    TClonesArray& clref = *fHitCA;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BGTPCHitData(x, y, z, longWidth, energy);
}

ClassImp(R3BGTPCCal2Hit)
