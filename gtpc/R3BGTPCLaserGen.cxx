/*******************************************************************************
 * Copyright (C) 2024 GSI Helmholtzzentrum für Schwerionenforschung GmbH       *
 * Copyright (C) 2024-2025 Members of the R3B Collaboration                    *
 *                                                                             *
 *             This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,            *
 *                   copied verbatim in the file "LICENSE".                    *
 *                                                                             *
 *  In applying this license GSI does not waive the privileges and immunities  *
 *  granted to it by virtue of its status as an Intergovernmental Organization *
 *  or submit itself to any jurisdiction.                                      *
 ********************************************************************************/

#include "R3BGTPCLaserGen.h"
#include "R3BMCTrack.h"

#include "TClonesArray.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

#include "R3BGladFieldMap.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include <TH2D.h>
using namespace std;

R3BGTPCLaserGen::R3BGTPCLaserGen()
    : FairTask("R3BGTPCLaserGen")
    , fGTPCGeoPar(nullptr)
    , fGTPCGasPar(nullptr)
    , fGTPCElecPar(nullptr)
    , fGTPCPointsCA(nullptr)
    , fGTPCProjPointCA(nullptr)
    , fGTPCCalDataCA(nullptr)
    , MCTrackCA(nullptr)
{
    // ALL UNITS IN cm, ns, V/cm, Tesla and GeV
    fAlpha = 0.; // deg
    fBeta = 0.;  // deg
    fXIn = 0.;   // cm
    fYIn = 0.;   // cm
    fZIn = 0.;   // cm
    fMaxLength = 0.;
    fOffsetX = 0.; // cm
    fOffsetZ = 0.; // cm
    outputMode = 0;
    fTPCMap = std::make_shared<R3BGTPCMap>();
}

R3BGTPCLaserGen::~R3BGTPCLaserGen()
{
    if (fGTPCPointsCA)
    {
        fGTPCPointsCA->Delete();
        delete fGTPCPointsCA;
    }
    if (fGTPCCalDataCA)
    {
        fGTPCCalDataCA->Delete();
        delete fGTPCCalDataCA;
    }
    if (fGTPCProjPointCA)
    {
        fGTPCProjPointCA->Delete();
        delete fGTPCProjPointCA;
    }
    if (MCTrackCA)
    {
        MCTrackCA->Delete();
        delete MCTrackCA;
    }
}

void R3BGTPCLaserGen::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(fatal) << "R3BGTPCLaserGen::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(fatal) << "R3BGTPCLaserGen::SetParContainers: No runtime database";
        return;
    }
    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(fatal) << "R3BGTPCLaserGen::SetParContainers: No R3BGTPCGeoPar";
        return;
    }
    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(fatal) << "R3BGTPCLaserGen::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(fatal) << "R3BGTPCLaserGen::SetParContainers: No R3BGTPCElecPar";
        return;
    }
}

void R3BGTPCLaserGen::SetParameter()
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

InitStatus R3BGTPCLaserGen::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(fatal) << "R3BGTPCLaserGen::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(fatal) << "R3BGTPCLaserGen::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPointsCA = (TClonesArray*)ioman->GetObject("GTPCPoint");
    // Input: TClonesArray of R3BMCTrack
    if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    {
        LOG(fatal) << "R3BMCTrack::Init No MCTrack!";
        return kFATAL;
    }

    MCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");
    // fGTPCProjPointCA = new TClonesArray("R3BGTPCProjPoint");
    // ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPointCA, kTRUE);

    if (outputMode == 0)
    { // Output: TClonesArray of R3BGTPCCalData
        fGTPCCalDataCA = new TClonesArray("R3BGTPCCalData");
        ioman->Register("GTPCCalData", GetName(), fGTPCCalDataCA, kTRUE);
    }
    else if (outputMode == 1)
    { // Output: TClonesArray of R3BGTPCProjPoint
        fGTPCProjPointCA = new TClonesArray("R3BGTPCProjPoint");
        ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPointCA, kTRUE);
    }

    SetParameter();

    fTPCMap->GeneratePadPlane();
    fPadPlane = fTPCMap->GetPadPlane();

    if (fPadPlane == NULL)
    {
        std::cout << " R3BGTPCProjector::Init() error! - Could not retrieve pad "
                     "plane. Exiting..."
                  << endl;
        return kERROR;
    }

    return kSUCCESS;
}

InitStatus R3BGTPCLaserGen::ReInit()
{
    SetParContainers();
    SetParameter();
    return kSUCCESS;
}

void R3BGTPCLaserGen::SetDriftParameters(Double_t ion,
                                         Double_t driftv,
                                         Double_t tDiff,
                                         Double_t lDiff,
                                         Double_t fanoFactor)
{
    fEIonization = ion;       // [GeV] NOTUSED
    fDriftVelocity = driftv;  // [cm/ns]
    fTransDiff = tDiff;       // [cm^(-1/2)]
    fLongDiff = lDiff;        // [cm^(-1/2)]
    fFanoFactor = fanoFactor; // NOTUSED
}

void R3BGTPCLaserGen::SetLaserParameters(Double_t alpha,
                                         Double_t beta,
                                         Double_t x_in,
                                         Double_t y_in,
                                         Double_t z_in,
                                         Double_t maxLength)
{
    fAlpha = alpha;         // deg
    fBeta = beta;           // deg
    fXIn = x_in;            // cm
    fYIn = y_in;            // cm
    fZIn = z_in;            // cm
    fMaxLength = maxLength; // cm
                            // fPointDistance = pointDistance; // cm
}

void R3BGTPCLaserGen::SetNumberOfGeneratedElectrons(Double_t ele)
{
    fNumberOfGeneratedElectrons = ele; // Number of electrons to generate in each point of the test
}

void R3BGTPCLaserGen::Exec(Option_t*)
{
    SetParameter();

    if (outputMode == 0)
        fGTPCCalDataCA->Clear("C");
    if (outputMode == 1)
        fGTPCProjPointCA->Clear("C");

    LOG(info) << "R3BGTPCLaserGen: test";
    Int_t nPoints = fGTPCPointsCA->GetEntries();

    R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField();

    R3BGTPCPoint* aPoint;
    Double_t projX, projZ, projTime;
    Double_t timeBeforeDrift = 0.;
    Bool_t padFound = kFALSE;
    Double_t driftDistance;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID = 0;
    Int_t PDGCode = 0, MotherId = 0;
    Double_t Vertex_x0 = 0, Vertex_y0 = 0, Vertex_z0 = 0, Vertex_px0 = 0, Vertex_py0 = 0, Vertex_pz0 = 0;

    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPointsCA->At(i);
        evtID = aPoint->GetEventID();
        R3BMCTrack* Track = (R3BMCTrack*)MCTrackCA->At(aPoint->GetTrackID());
        PDGCode = Track->GetPdgCode();
        MotherId = Track->GetMotherId();
        Vertex_x0 = Track->GetStartX();
        Vertex_y0 = Track->GetStartY();
        Vertex_z0 = Track->GetStartZ();
        Vertex_px0 = Track->GetPx();
        Vertex_py0 = Track->GetPy();
        Vertex_pz0 = Track->GetPz();
    }

    // Using here m, V, T, s just to simplify in SI system...
    Double_t E_x = 0;
    Double_t E_z = 0;
    Double_t E_y = fDriftEField * 100;
    Double_t B_x = 0.;
    Double_t B_y = 0.;
    Double_t B_z = 0.;
    Double_t moduleB = 0;
    Double_t ele_x_init = 0;
    Double_t ele_y_init = 0;
    Double_t ele_z_init = 0;
    Double_t ele_x = 0;
    Double_t ele_y = 0;
    Double_t ele_z = 0;
    Double_t vDrift_x = 0;
    Double_t vDrift_y = 0;
    Double_t vDrift_z = 0;
    Double_t accDriftTime = 0;
    Double_t cteMult = 0;
    Double_t cteMod = 0;
    Double_t productEB = 0;
    Double_t sigmaLongStep;
    Double_t sigmaTransvStep;
    Double_t driftTimeStep;
    Double_t mu = 1.E+7 * fDriftVelocity / E_y; // [m2 s-1 V-1] TODO check value, move to parameter container

    // from create_tpc_geo_test.C (geo in file
    // R3BRoot/glad-tpc/geometry/gladTPC_test.geo.root)
    Double_t TargetOffsetX = fTargetOffsetX;
    Double_t TargetOffsetY = fTargetOffsetY;
    Double_t TargetOffsetZ = fTargetOffsetZ; // USING INSTEAD THE FIELD MAP DESPLACEMENT! MISMATCH
    Double_t TargetOffsetZ_FM = fTargetOffsetZ;

    // Those values seem not to be anywhere
    // TargetOffsetZ_FM = 263.4; // FIELD MAP DESPLACEMENT
    Double_t TargetAngle = 14. * TMath::Pi() / 180;
    fOffsetX = 0;
    fOffsetZ = 0;

    B_x = 0.1 * gladField->GetBx(0, 0, 163.4); // Field components return in [kG], moved to [T]
    B_y = 0.1 * gladField->GetBy(0, 0, 163.4);
    B_z = 0.1 * gladField->GetBz(0, 0, 163.4);
    cout << "Field for (0,0,163.4) in R3B coordinates or (0,0,0) in field map "
            "coordinates: "
         << B_x << " " << B_y << " " << B_z << " " << endl;
    B_x = 0.1 * gladField->GetBx(-7.2837383, -10,
                                 151.27782); // Field components return in [kG], moved to [T]
    B_y = 0.1 * gladField->GetBy(-7.2837383, -10, 151.27782);
    B_z = 0.1 * gladField->GetBz(-7.2837383, -10, 151.27782);
    cout << "Field for (-7.2837383, -10, 151.27782) in R3B coordinates or (-10, "
            "-10, -10) in field map coordinates: "
         << B_x << " " << B_y << " " << B_z << " " << endl;
    B_x = 0.1 * gladField->GetBx(-7.2837383, 10,
                                 151.27782); // Field components return in [kG], moved to [T]
    B_y = 0.1 * gladField->GetBy(-7.2837383, 10, 151.27782);
    B_z = 0.1 * gladField->GetBz(-7.2837383, 10, 151.27782);
    cout << "Field for (-7.2837383, 10, 151.27782) in R3B coordinates or (-10, "
            "10, -10) in field map coordinates: "
         << B_x << " " << B_y << " " << B_z << " " << endl;
    B_x = 0.1 * gladField->GetBx(12.122176, -10,
                                 156.11626); // Field components return in [kG], moved to [T]
    B_y = 0.1 * gladField->GetBy(12.122176, -10, 156.11626);
    B_z = 0.1 * gladField->GetBz(12.122176, -10, 156.11626);
    cout << "Field for (12.122176, -10, 156.11626) in R3B coordinates or (10, "
            "-10, -10) in field map coordinates: "
         << B_x << " " << B_y << " " << B_z << " " << endl;
    B_x = 0.1 * gladField->GetBx(12.122176, 10,
                                 156.11626); // Field components return in [kG], moved to [T]
    B_y = 0.1 * gladField->GetBy(12.122176, 10, 156.11626);
    B_z = 0.1 * gladField->GetBz(12.122176, 10, 156.11626);
    cout << "Field for (12.122176, 10, 156.11626) in R3B coordinates or (10, 10, "
            "-10) in field map coordinates: "
         << B_x << " " << B_y << " " << B_z << " " << endl;

    // Express the angles in rad
    fAlpha *= TMath::Pi() / 180.;
    fBeta *= TMath::Pi() / 180.;

    // Calculate the radius where the laser scapes the TPC
    Double_t rX = 2 * fHalfSizeTPC_X / cos(fBeta) / sin(fAlpha);
    Double_t rY, rZ;

    if (fAlpha > 0) // The laser only can scape through the top
    {
        rY = (2 * fHalfSizeTPC_Y - fYIn) / sin(fBeta);
    }
    else // The laser only can scape through the bottom
    {
        rY = (fYIn) / sin(fBeta);
    }

    if ((fBeta < TMath::Pi() / 2) && (fBeta > 0.))
    {
        rZ = (2 * fHalfSizeTPC_Z - fZIn) / cos(fBeta) / cos(fAlpha);
    }
    else
    {
        rZ = (fZIn) / cos(fBeta) / cos(fAlpha);
    }

    Double_t rads[3] = { rX, rY, rZ };
    Double_t rMin = TMath::MinElement(3, rads);

    TRandom3 rndGen;
    for (Int_t k = 0; k < fMaxLength; k++)
    {
        Double_t r = rndGen.Uniform(0, rMin);

        // Parametrize the straight line with beta and alpha angles
        Double_t xval = r * cos(fBeta) * sin(fAlpha);
        Double_t zval = fZIn + r * cos(fBeta) * cos(fAlpha);
        Double_t yval = fYIn + r * sin(fBeta);

        ele_y_init = yval - fHalfSizeTPC_Y;
        ele_x_init = cos(-TargetAngle) * (xval) + sin(-TargetAngle) * (zval);
        ele_z_init = TargetOffsetZ_FM - fHalfSizeTPC_Z - sin(-TargetAngle) * (xval) + cos(-TargetAngle) * (zval);

        // taken the driftDistance for the calculation of the sigmaLong and
        // sigmaTrans
        driftDistance = ele_y + fHalfSizeTPC_Y;
        sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);

        for (Int_t ele = 0; ele < fNumberOfGeneratedElectrons; ele++)
        {
            ele_x = ele_x_init;
            ele_y = ele_y_init;
            ele_z = ele_z_init;

            accDriftTime = timeBeforeDrift;
            driftTimeStep = fDriftTimeStep;
            LOG(debug) << "R3BGTPCLaserGen::Exec, INITIAL VALUES: timeBeforeDrift=" << accDriftTime << " [ns]"
                       << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";

            while (ele_y > -fHalfSizeTPC_Y) // while not reaching the pad plane [cm]
            {

                B_x = 0.1 * gladField->GetBx(ele_x, ele_y,
                                             ele_z); // Field components return in [kG], moved to [T]
                B_y = 0.1 * gladField->GetBy(ele_x, ele_y, ele_z);
                B_z = 0.1 * gladField->GetBz(ele_x, ele_y, ele_z);

                moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // in [T]
                cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // SI
                cteMult = mu * cteMod;                                    // SI

                // assuming only vertical electric field in the next four lines
                productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; SI

                vDrift_x = cteMult * (mu * (E_y * B_z) + mu * mu * productEB * B_x); // cte * (Ex + mu*(E_y*B_z-E_z*B_y)
                                                                                     // + mu*mu*productEB*B_x); SI
                vDrift_y = cteMult * (E_y + mu * mu * productEB * B_y); // cte * (Ey + mu*(E_z*B_x-E_x*B_z) +
                                                                        // mu*mu*productEB*B_y); SI
                vDrift_z =
                    cteMult * (mu * (-E_y * B_x) + mu * mu * productEB * B_z); // cte * (Ez + mu*(E_x*B_y-E_y*B_x) +
                                                                               // mu*mu*productEB*B_z); SI

                LOG(debug) << "R3BGTPCLaserGen::Exec, timeBeforeDrift=vDrift_x=" << vDrift_x << " vDrift_y=" << vDrift_y
                           << " vDrift_z=" << vDrift_z << " [m/s]";

                // adjusting the last step before the pad plane
                if (ele_y - 1.E-7 * vDrift_y * driftTimeStep < -fHalfSizeTPC_Y)
                    driftTimeStep = (ele_y + fHalfSizeTPC_Y) / (1.E-7 * vDrift_y); // vDrift back to [cm/ns]

                // reducing sigmaTransv (see
                // http://web.ift.uib.no/~lipniack/detectors/lecture5/detector5.pdf) as
                // B~B_y and E=E_y, let's simplify and apply the reduction to the
                // transversal coefficient without projections
                sigmaTransvStep =
                    sqrt(driftTimeStep * 2 * fTransDiff * cteMod); // should be reduced by the factor cteMod=cteMult/mu
                sigmaLongStep =
                    sqrt(driftTimeStep * 2 * fLongDiff); // should be the same scaled to the length of the step
                ele_x = gRandom->Gaus(ele_x + 1.E-7 * vDrift_x * driftTimeStep,
                                      sigmaTransvStep); // vDrift back to [cm/ns]
                ele_y = gRandom->Gaus(ele_y - 1.E-7 * vDrift_y * driftTimeStep, sigmaLongStep);
                ele_z = gRandom->Gaus(ele_z + 1.E-7 * vDrift_z * driftTimeStep, sigmaTransvStep);
                accDriftTime = accDriftTime + driftTimeStep;

                LOG(debug) << "R3BGTPCLaserGen::Exec, accDriftTime=" << accDriftTime << " [ns]"
                           << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            }

            projTime = accDriftTime;

            // obtain padID for projX, projZ (simple algorithm)
            // 1) Fill an histogram with histoBins (X-axis) and histoBins1 (Y-axis)
            // 2) Get the ID of the bin filled in each event
            // 3) This ID can be used to reconstruct the histogram by inverting the
            // process

            projX = +cos(TargetAngle) * ele_x + sin(TargetAngle) * (ele_z - (TargetOffsetZ_FM - fHalfSizeTPC_Z));
            projZ = -sin(TargetAngle) * ele_x + cos(TargetAngle) * (ele_z - (TargetOffsetZ_FM - fHalfSizeTPC_Z));

            // Removing electrons out of pad plane limits
            if (projZ < fOffsetZ || projZ > fOffsetZ + 2 * fHalfSizeTPC_Z || projX < fOffsetX ||
                projX > fOffsetX + 2 * fHalfSizeTPC_X)
                continue;

            // Adding -1 to get padID between 0 - 5631
            Int_t padID = fPadPlane->Fill((projZ - fOffsetZ) * 10.0,
                                          (projX - fOffsetX) * 10.0) -
                          1; // in mm for the padID

            // If returns negative padID means its filling overflow/underflow bins
            // Maybe error in the conditionals projX and projZ above
            if (padID < 0 || padID > 5631)
            {
                LOG(warn) << "R3BGTPCLangevin::Exec No-valid padID" << endl;
                continue;
            }

            if (outputMode == 0)
            { // Output: TClonesArray of R3BGTPCCalData
                Int_t nCalData = fGTPCCalDataCA->GetEntriesFast();
                for (Int_t pp = 0; pp < nCalData; pp++)
                {
                    if (((R3BGTPCCalData*)fGTPCCalDataCA->At(pp))->GetPadId() == padID)
                    {
                        // already existing R3BGTPCCalData... add time and electron
                        projTime = projTime / fTimeBinSize; // moving from ns to binsize
                        if (projTime < 0)
                            projTime = 0; // Fills (first) underflow bin
                        else if (projTime > 511)
                            projTime = 511; // Fills (last) overflow bin
                        ((R3BGTPCCalData*)fGTPCCalDataCA->At(pp))->SetADC(projTime);
                        padFound = kTRUE;
                        break;
                    }
                }
                if (!padFound)
                {
                    std::vector<UShort_t> adc(512, 0);
                    projTime = projTime / fTimeBinSize; // moving from ns to binsize
                    if (projTime < 0)
                        projTime = 0; // Fills (first) underflow bin
                    else if (projTime > 511)
                        projTime = 511; // Fills (last) overflow bin
                    adc.at(projTime)++;
                    new ((*fGTPCCalDataCA)[nCalData]) R3BGTPCCalData(padID, adc);
                }
                padFound = kFALSE;
            }

            else if (outputMode == 1)
            { // Output: TClonesArray of R3BGTPCProjPoint
                Int_t nProjPoints = fGTPCProjPointCA->GetEntriesFast();
                for (Int_t pp = 0; pp < nProjPoints; pp++)
                {
                    if (((R3BGTPCProjPoint*)fGTPCProjPointCA->At(pp))->GetVirtualPadID() == padID)
                    {
                        // already existing R3BGTPCProjPoint... add time and electron
                        ((R3BGTPCProjPoint*)fGTPCProjPointCA->At(pp))->AddCharge();
                        ((R3BGTPCProjPoint*)fGTPCProjPointCA->At(pp))
                            ->SetTimeDistr(projTime / fTimeBinSize, 1); // micros
                        padFound = kTRUE;
                        break;
                    }
                }
                if (!padFound)
                {
                    new ((*fGTPCProjPointCA)[nProjPoints]) R3BGTPCProjPoint(padID,
                                                                            projTime / fTimeBinSize, // micros
                                                                            1,
                                                                            evtID,
                                                                            PDGCode,
                                                                            MotherId,
                                                                            Vertex_x0,
                                                                            Vertex_y0,
                                                                            Vertex_z0,
                                                                            Vertex_px0,
                                                                            Vertex_py0,
                                                                            Vertex_pz0);
                }
                padFound = kFALSE;
            }
        }
    }

    // LOG(info) << "R3BGTPCLaserGen: produced " << fGTPCProjPoint->GetEntries()
    // << " projPoints";
}

void R3BGTPCLaserGen::Finish() {}

ClassImp(R3BGTPCLaserGen)
