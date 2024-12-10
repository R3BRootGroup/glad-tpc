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

#include "R3BGTPCLangevin.h"
#include "R3BMCTrack.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "R3BGladFieldMap.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

#include "TF1.h"
using namespace std;

R3BGTPCLangevin::R3BGTPCLangevin()
    : FairTask("R3BGTPCLangevin")
    , fGTPCPointsCA(NULL)
    , fGTPCCalDataCA(NULL)
    , fGTPCProjPointCA(NULL)
    , fMCTrackCA(NULL)
{
    fEIonization = 0;
    fDriftVelocity = 0;
    fTransDiff = 0;
    fLongDiff = 0;
    fFanoFactor = 0;
    fSizeOfVirtualPad = 0;
    fHalfSizeTPC_X = 0.;
    fHalfSizeTPC_Y = 0.;
    fHalfSizeTPC_Z = 0.;
    fSizeOfVirtualPad = 0;
    fTimeBinSize = 0;
    fDriftEField = 0.;
    fDriftTimeStep = 0.;
    fDetectorType = 0;
    outputMode = 0;
    fTPCMap = std::make_shared<R3BGTPCMap>();
}

R3BGTPCLangevin::~R3BGTPCLangevin()
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
    if (fMCTrackCA)
    {
        fMCTrackCA->Delete();
        delete fMCTrackCA;
    }
}

void R3BGTPCLangevin::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No runtime database";
        return;
    }
    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGeoPar";
        return;
    }
    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(fatal) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCElecPar";
        return;
    }
}

void R3BGTPCLangevin::SetParameter()
{
    // From gas properties
    fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]-> typical value for a
                                                      // gas detector tens of eV
    fDriftVelocity = fGTPCGasPar->GetDriftVelocity(); // [cm/ns]-> Minos TPC with a Efield=152 V/cm
    fTransDiff = fGTPCGasPar->GetTransDiff();         // [cm^2/ns]?
    fLongDiff = fGTPCGasPar->GetLongDiff();           // [cm^2/ns]?
    fFanoFactor = fGTPCGasPar->GetFanoFactor();
    // From geo properties
    fHalfSizeTPC_X = fGTPCGeoPar->GetActiveRegionx() / 2.;
    fHalfSizeTPC_Y = fGTPCGeoPar->GetActiveRegiony() / 2.;
    fHalfSizeTPC_Z = fGTPCGeoPar->GetActiveRegionz() / 2.;
    fSizeOfVirtualPad = fGTPCGeoPar->GetPadSize(); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    fDetectorType = fGTPCGeoPar->GetDetectorType();
    fOffsetX = fGTPCGeoPar->GetGladOffsetX(); // X offset [cm]
    fOffsetZ = fGTPCGeoPar->GetGladOffsetZ(); // Z offset [cm]
    // From electronic properties
    fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/cm
    fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift params calculation
    fTimeBinSize = fGTPCElecPar->GetTimeBinSize();     // time step for drift params calculation
}

InitStatus R3BGTPCLangevin::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(fatal) << "R3BGTPCLangevin::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(fatal) << "R3BGTPCLangevin::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPointsCA = (TClonesArray*)ioman->GetObject("GTPCPoint");
    // Input: TClonesArray of R3BMCTrack
    if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    {
        LOG(fatal) << "R3BMCTrack::Init No MCTrack!";
        return kFATAL;
    }

    fMCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");

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

    // Pad plane generation
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

InitStatus R3BGTPCLangevin::ReInit()
{
    SetParContainers();
    SetParameter();
    return kSUCCESS;
}

void R3BGTPCLangevin::Exec(Option_t*)
{
    if (outputMode == 0)
        fGTPCCalDataCA->Clear("C");
    if (outputMode == 1)
        fGTPCProjPointCA->Clear("C");

    Int_t nPoints = fGTPCPointsCA->GetEntries();
    LOG(info) << "R3BGTPCLangevin: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(info) << "Not enough hits for digitization! (<2)";
        return;
    }

    R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField();
    R3BGTPCPoint* aPoint;
    Int_t presentTrackID = -10; // control of the point trackID
    Double_t xPre, yPre, zPre;
    Double_t xPost, yPost, zPost;
    Double_t projX, projZ, projTime;
    Double_t energyDep = 0.;
    Double_t timeBeforeDrift = 0.;
    Bool_t readyToProject = kFALSE;
    Bool_t padFound = kFALSE;
    Int_t electrons = 0;
    Int_t flucElectrons = 0;
    Int_t generatedElectrons = 0;
    Double_t stepX, stepY, stepZ;
    Double_t driftDistance, driftTime;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID = 0;
    Int_t PDGCode = 0, MotherId = 0;
    Double_t Vertex_x0 = 0, Vertex_y0 = 0, Vertex_z0 = 0, Vertex_px0 = 0, Vertex_py0 = 0, Vertex_pz0 = 0;
    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPointsCA->At(i);
        evtID = aPoint->GetEventID();
        if (aPoint->GetTrackStatus() == 11000 || aPoint->GetTrackStatus() == 10010010 ||
            aPoint->GetTrackStatus() == 10010000 || aPoint->GetTrackStatus() == 10011000)
        {
            // entering the gas volume or new track inside the gas (is 10010010 or
            // 10010000??)
            presentTrackID = aPoint->GetTrackID();
            // from gMC->TrackPosition() -> current position in the master reference
            // frame of the track
            xPre = aPoint->GetX();
            yPre = aPoint->GetY();
            zPre = aPoint->GetZ();
            R3BMCTrack* Track = (R3BMCTrack*)fMCTrackCA->At(presentTrackID);
            PDGCode = Track->GetPdgCode();
            MotherId = Track->GetMotherId();
            Vertex_x0 = Track->GetStartX();
            Vertex_y0 = Track->GetStartY();
            Vertex_z0 = Track->GetStartZ();
            Vertex_px0 = Track->GetPx();
            Vertex_py0 = Track->GetPy();
            Vertex_pz0 = Track->GetPz();
            readyToProject = kTRUE;
            continue; // no energy deposited in this point, just taking in entrance
                      // coordinates
                      // NOTE: the entering points deposit no energy but is used as start point
                      // for the calculation of the track portion where energy is used in a
                      // regular electron ionization below
        }
        else
        { // any other case, that is, other than entering the volume
            if (presentTrackID != aPoint->GetTrackID())
            { // track was not entering the volume in a
              // previous point, what somehow it is in! :‑O
                LOG(fatal) << "R3BGTPCLangevin::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            { // track somehow exited the gas volume or
              // dissappeared in a previous point :‑O
                LOG(fatal) << "R3BGTPCLangevin::Exec: Problem 3 in point logic";
                break;
            }
            if (aPoint->GetTrackStatus() == 10100 || aPoint->GetTrackStatus() == 1000000)
            { // exiting the gas volume or dissappearing
                readyToProject = kFALSE;
            }
            // again from gMC->TrackPosition() for next point position
            xPost = aPoint->GetX();
            yPost = aPoint->GetY();
            zPost = aPoint->GetZ();
            energyDep = aPoint->GetEnergyLoss();
            timeBeforeDrift = aPoint->GetTime(); // ns
        }
        electrons = energyDep / fEIonization;
        // electron number fluctuates as the square root of
        // the Fano factor times the number of electrons
        flucElectrons = pow(fFanoFactor * electrons, 0.5);
        generatedElectrons = gRandom->Gaus(electrons, flucElectrons); // generated electrons

        // step in each direction for an homogeneous electron creation position
        // along the track
        stepX = (xPost - xPre) / generatedElectrons;
        stepY = (yPost - yPre) / generatedElectrons;
        stepZ = (zPost - zPre) / generatedElectrons;

        // taken a mean driftDistance for the calculation of the sigmaLong and
        // sigmaTrans improve it by making the calculation individual for each
        // electron, but probably slower
        Double_t yApprox = (yPost + yPre) / 2;
        driftDistance = yApprox + fHalfSizeTPC_Y;
        sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);

        Double_t E_x = 0;
        Double_t E_z = 0;
        Double_t E_y = fDriftEField; // in V/m
        Double_t B_x = 0;
        Double_t B_y = 0;
        Double_t B_z = 0;
        Double_t moduleB = 0;
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
        Double_t recoverDriftTimeStep = 0.;
        Bool_t recoverStep = kTRUE;

        Double_t mu = fDriftVelocity / E_y; // [cm^2 ns^-1 V^-1]

        for (Int_t ele = 1; ele <= generatedElectrons; ele++)
        {
            // For a single electron
            ele_x = xPre + stepX * ele; // homogeneous electron creation along the step [cm]
            ele_y = yPre + stepY * ele;
            ele_z = zPre + stepZ * ele;
            accDriftTime = timeBeforeDrift;

            LOG(debug) << "R3BGTPCLangevin::Exec, INITIAL VALUES: timeBeforeDrift=" << accDriftTime << " [ns]"
                       << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            // cout   << "R3BGTPCLangevin::Exec, INITIAL VALUES: timeBeforeDrift=" <<
            // accDriftTime << " [ns]"
            //                  << " ele_x=" << ele_x << " ele_y=" << ele_y << "
            //                  ele_z=" << ele_z << " [cm]";
            while (ele_y > -fHalfSizeTPC_Y)
            { // while not reaching the pad plane [cm]
                B_x = 1e4 * gladField->GetBx(ele_x,
                                             ele_y,
                                             ele_z); // Field components return in [kG],
                                                     // moved to [V ns cm^-2]
                B_y = 1e4 * gladField->GetBy(ele_x, ele_y, ele_z);
                B_z = 1e4 * gladField->GetBz(ele_x, ele_y, ele_z);
                // std::cout << "MAGNETIC FIELD || Bx: "<<B_x<< " By: "<<B_y<<" Bz:
                // "<<B_z<< '\n';

                moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // in [V ns cm^-2]
                cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // adimensional
                cteMult = mu * cteMod;                                    // [cm^2 V^-1 ns^-1]

                // assuming only vertical electric field in the next four lines
                productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; [V^2 ns cm^-3]

                vDrift_x = cteMult * (mu * (E_y * B_z) + mu * mu * productEB * B_x); // cte * (Ex + mu*(E_y*B_z-E_z*B_y)
                                                                                     // + mu*mu*productEB*B_x); [cm/ns]
                vDrift_y = cteMult * (E_y + mu * mu * productEB * B_y); // cte * (Ey + mu*(E_z*B_x-E_x*B_z) +
                                                                        // mu*mu*productEB*B_y); [cm/ns]
                vDrift_z =
                    cteMult * (mu * (-E_y * B_x) + mu * mu * productEB * B_z); // cte * (Ez + mu*(E_x*B_y-E_y*B_x) +
                                                                               // mu*mu*productEB*B_z); [cm/ns]

                LOG(debug) << "R3BGTPCLangevin::Exec, DRIFT VELOCITIES: vDrift_x=" << vDrift_x
                           << " vDrift_y=" << vDrift_y << " vDrift_z=" << vDrift_z << " [cm/ns]";
                // adjusting the last step before the pad plane
                if (ele_y - vDrift_y * fDriftTimeStep < -fHalfSizeTPC_Y)
                {
                    if (recoverStep)
                        recoverDriftTimeStep = fDriftTimeStep;
                    recoverStep = kFALSE;
                    fDriftTimeStep = (ele_y + fHalfSizeTPC_Y) / vDrift_y;
                }

                // reducing sigmaTransv (see
                // http://web.ift.uib.no/~lipniack/detectors/lecture5/detector5.pdf) as
                // B~B_y and E=E_y, let's simplify and apply the reduction to the
                // transversal coefficient without projections
                sigmaTransvStep =
                    sqrt(fDriftTimeStep * 2 * fTransDiff * cteMod); // should be reduced by the factor cteMod=cteMult/mu
                sigmaLongStep =
                    sqrt(fDriftTimeStep * 2 * fLongDiff); // should be the same scaled to the length of the step
                ele_x = gRandom->Gaus(ele_x + vDrift_x * fDriftTimeStep,
                                      sigmaTransvStep); // [cm]
                ele_y = gRandom->Gaus(ele_y - vDrift_y * fDriftTimeStep,
                                      sigmaLongStep); // [cm]
                ele_z = gRandom->Gaus(ele_z + vDrift_z * fDriftTimeStep,
                                      sigmaTransvStep);       // [cm]
                accDriftTime = accDriftTime + fDriftTimeStep; //[ns]

                // TODO!!! CHECK THE NEGATIVE sign in the y directions three lines
                // above... Could it be symmetric with the others (+) in case the
                // electric field is negative in Y? Does it change other cross terms?
                // Which one is correct?

                LOG(debug) << "R3BGTPCLangevin::Exec, NEW VALUES: accDriftTime=" << accDriftTime << " [ns]"
                           << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            }
            if (recoverDriftTimeStep)
            {
                fDriftTimeStep = recoverDriftTimeStep;
                recoverDriftTimeStep = 0.;
                recoverStep = kTRUE;
            }
            // FINAL RESULT: X,Z position and time of the electron after Langevin
            // calculation:
            projX = ele_x;
            projZ = ele_z;
            projTime = accDriftTime;

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
        xPre = xPost;
        yPre = yPost;
        zPre = zPost;
    }
    if (outputMode == 0)
        LOG(info) << "R3BGTPCLangevin: produced " << fGTPCCalDataCA->GetEntries() << " R3BGTPCcalData(s)";
    if (outputMode == 1)
        LOG(info) << "R3BGTPCLangevin: produced " << fGTPCProjPointCA->GetEntries() << " R3BGTPCProjPoint(s)";
}

void R3BGTPCLangevin::Finish() {}

ClassImp(R3BGTPCLangevin)
