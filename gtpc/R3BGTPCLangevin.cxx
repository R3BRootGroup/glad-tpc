/******************************************************************************
 *   Copyright (C) 2020 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2020 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
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
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No runtime database";
        return;
    }
    fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!fGTPCGeoPar)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGeoPar";
        return;
    }
    fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!fGTPCGasPar)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCGasPar";
        return;
    }
    fGTPCElecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!fGTPCElecPar)
    {
        LOG(FATAL) << "R3BGTPCLangevin::SetParContainers: No R3BGTPCElecPar";
        return;
    }
}

void R3BGTPCLangevin::SetParameter()
{
    // From gas properties
    fEIonization = fGTPCGasPar->GetEIonization();     // [GeV]-> typical value for a gas detector tens of eV
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
    // From electronic properties
    fDriftEField = fGTPCElecPar->GetDriftEField();     // drift E field in V/m
    fDriftTimeStep = fGTPCElecPar->GetDriftTimeStep(); // time step for drift params calculation
    fTimeBinSize = fGTPCElecPar->GetTimeBinSize();     // time step for drift params calculation
}

InitStatus R3BGTPCLangevin::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(FATAL) << "R3BGTPCLangevin::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(FATAL) << "R3BGTPCLangevin::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPointsCA = (TClonesArray*)ioman->GetObject("GTPCPoint");
    // Input: TClonesArray of R3BMCTrack
    if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    {
        LOG(FATAL) << "R3BMCTrack::Init No MCTrack!";
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
    LOG(INFO) << "R3BGTPCLangevin: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(INFO) << "Not enough hits for digitization! (<2)";
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
    Int_t evtID;
    Int_t PDGCode, MotherId;
    Double_t Vertex_x0, Vertex_y0, Vertex_z0, Vertex_px0, Vertex_py0, Vertex_pz0;
    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPointsCA->At(i);
        evtID = aPoint->GetEventID();
        if (aPoint->GetTrackStatus() == 11000 || aPoint->GetTrackStatus() == 10010010 ||
            aPoint->GetTrackStatus() == 10010000 || aPoint->GetTrackStatus() == 10011000)
        {
            // entering the gas volume or new track inside the gas (is 10010010 or 10010000??)
            presentTrackID = aPoint->GetTrackID();
            // from gMC->TrackPosition() -> current position in the master reference frame of the track
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
            continue; // no energy deposited in this point, just taking in entrance coordinates
            // NOTE: the entering points deposit no energy but is used as start point for the calculation
            // of the track portion where energy is used in a regular electron ionization below
        }
        else
        { // any other case, that is, other than entering the volume
            if (presentTrackID != aPoint->GetTrackID())
            { // track was not entering the volume in a previous point, what somehow it is in! :‑O
                LOG(FATAL) << "R3BGTPCLangevin::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            { // track somehow exited the gas volume or dissappeared in a previous point :‑O
                LOG(FATAL) << "R3BGTPCLangevin::Exec: Problem 3 in point logic";
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

        // step in each direction for an homogeneous electron creation position along the track
        stepX = (xPost - xPre) / generatedElectrons;
        stepY = (yPost - yPre) / generatedElectrons;
        stepZ = (zPost - zPre) / generatedElectrons;

        // taken a mean driftDistance for the calculation of the sigmaLong and sigmaTrans
        // improve it by making the calculation individual for each electron, but probably slower
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
            ele_x = xPre + stepX * ele; // homogeneous electron creation along the step [cm]
            ele_y = yPre + stepY * ele;
            ele_z = zPre + stepZ * ele;
            accDriftTime = timeBeforeDrift;

            LOG(DEBUG) << "R3BGTPCLangevin::Exec, INITIAL VALUES: timeBeforeDrift=" << accDriftTime << " [ns]"
                       << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            // cout   << "R3BGTPCLangevin::Exec, INITIAL VALUES: timeBeforeDrift=" << accDriftTime << " [ns]"
            //                  << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
            while (ele_y > -fHalfSizeTPC_Y)
            { // while not reaching the pad plane [cm]
                B_x = 1e-14 *
                      gladField->GetBx(ele_x, ele_y, ele_z); // Field components return in [kG], moved to [V ns cm^-2]
                B_y = 1e-14 * gladField->GetBy(ele_x, ele_y, ele_z);
                B_z = 1e-14 * gladField->GetBz(ele_x, ele_y, ele_z);

                moduleB = TMath::Sqrt(B_x * B_x + B_y * B_y + B_z * B_z); // in [V ns cm^-2]
                cteMod = 1 / (1 + mu * mu * moduleB * moduleB);           // adimensional
                cteMult = mu * cteMod;                                    // [cm^2 V^-1 ns^-1]

                // assuming only vertical electric field in the next four lines
                productEB = E_y * B_y; // E_x*B_x + E_y*B_y + E_z*B_z; [V^2 ns cm^-3]

                vDrift_x = cteMult * (mu * (E_y * B_z) +
                                      mu * mu * productEB *
                                          B_x); // cte * (Ex + mu*(E_y*B_z-E_z*B_y) + mu*mu*productEB*B_x); [cm/ns]
                vDrift_y =
                    cteMult * (E_y + mu * mu * productEB *
                                         B_y); // cte * (Ey + mu*(E_z*B_x-E_x*B_z) + mu*mu*productEB*B_y); [cm/ns]
                vDrift_z = cteMult * (mu * (-E_y * B_x) +
                                      mu * mu * productEB *
                                          B_z); // cte * (Ez + mu*(E_x*B_y-E_y*B_x) + mu*mu*productEB*B_z); [cm/ns]

                LOG(DEBUG) << "R3BGTPCLangevin::Exec, timeBeforeDrift=vDrift_x=" << vDrift_x << " vDrift_y=" << vDrift_y
                           << " vDrift_z=" << vDrift_z << " [cm/ns]";
                // adjusting the last step before the pad plane
                if (ele_y - vDrift_y * fDriftTimeStep < -fHalfSizeTPC_Y)
                {
                    if (recoverStep)
                        recoverDriftTimeStep = fDriftTimeStep;
                    recoverStep = kFALSE;
                    fDriftTimeStep = (ele_y + fHalfSizeTPC_Y) / vDrift_y;
                }

                // reducing sigmaTransv (see http://web.ift.uib.no/~lipniack/detectors/lecture5/detector5.pdf)
                // as B~B_y and E=E_y, let's simplify and apply the reduction to the transversal coefficient without
                // projections
                sigmaTransvStep =
                    sqrt(fDriftTimeStep * 2 * fTransDiff * cteMod); // should be reduced by the factor cteMod=cteMult/mu
                sigmaLongStep =
                    sqrt(fDriftTimeStep * 2 * fLongDiff); // should be the same scaled to the length of the step
                ele_x = gRandom->Gaus(ele_x + vDrift_x * fDriftTimeStep, sigmaTransvStep); // [cm]
                ele_y = gRandom->Gaus(ele_y - vDrift_y * fDriftTimeStep, sigmaLongStep);   // [cm]
                ele_z = gRandom->Gaus(ele_z + vDrift_z * fDriftTimeStep, sigmaTransvStep); // [cm]
                accDriftTime = accDriftTime + fDriftTimeStep;                              //[ns]

                // TODO!!! CHECK THE NEGATIVE sign in the y directions three lines above...
                // Could it be symmetric with the others (+) in case the electric field is negative in Y?
                // Does it change other cross terms? Which one is correct?

                LOG(DEBUG) << "R3BGTPCLangevin::Exec, accDriftTime=" << accDriftTime << " [ns]"
                           << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]";
                //  cout << "R3BGTPCLangevin::Exec, accDriftTime=" << accDriftTime << " [ns]"
                //                  << " ele_x=" << ele_x << " ele_y=" << ele_y << " ele_z=" << ele_z << " [cm]" <<
                //                  endl;
            }
            if (recoverDriftTimeStep)
            {
                fDriftTimeStep = recoverDriftTimeStep;
                recoverDriftTimeStep = 0.;
                recoverStep = kTRUE;
            }
            // FINAL RESULT: X,Z position and time of the electron after Langevin calculation:
            projX = ele_x;
            projZ = ele_z;
            projTime = accDriftTime;
            // obtain padID for projX, projZ (simple algorithm)
            // the algorithm assigns a pad number which depends on the XZ size of the chamber,
            // according to the fSizeOfVirtualPad parameter: if it is 1, the pad size is cm^2
            // and padID goes from 0 to 2*fHalfSizeTPC_X in the first row (ZOffset),
            // from 2*fHalfSizeTPC_X to 4*fHalfSizeTPC_X in the second row (ZOffset+1), ...
            // if fSizeOfVirtualPad=0.1, then padID goes from 0 to 20*fHalfSizeTPC_X for (Z~200.0),
            // from 20*fHalfSizeTPC_X to 40*fHalfSizeTPC_X  (Z~200.0), ...
            // Avoid first moving out of the virtual pad plane limits
            // ZOffset- z of the first pad row in the laboratory frame
            double ZOffset = 272.7; ////TODO!!! WHY ARE THOSE OFFSETS NEEDED HERE? WHY NOT PARAMETERS!!!!???????????
            // XOffset-y of the first pad column in the laboratory frame
            double XOffset = 5.8; ////TODO!!! WHY ARE THOSE OFFSETS NEEDED HERE? WHY NOT PARAMETERS!!!!???????????
            if (projZ < ZOffset)
                projZ = ZOffset;
            if (projZ > ZOffset + 2 * fHalfSizeTPC_Z)
                projZ = ZOffset + 2 * fHalfSizeTPC_Z;
            if (projX < XOffset)
                projX = XOffset;
            if (projX > XOffset + 2 * fHalfSizeTPC_X)
                projX = XOffset + 2 * fHalfSizeTPC_X;
            Int_t padID;
            if (fDetectorType == 1)
                padID = (44) * (Int_t)((projZ - ZOffset) / 0.2) + (Int_t)((projX - XOffset) / 0.2); // 2mm
            else
                padID = (2 * fHalfSizeTPC_X * fSizeOfVirtualPad) * (Int_t)((projZ - ZOffset) * fSizeOfVirtualPad) +
                        (Int_t)((projX - XOffset) * fSizeOfVirtualPad); // FULL HYDRA padplane has not been decided yet

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
        LOG(INFO) << "R3BGTPCLangevin: produced " << fGTPCCalDataCA->GetEntries() << " R3BGTPCcalData(s)";
    if (outputMode == 1)
        LOG(INFO) << "R3BGTPCLangevin: produced " << fGTPCProjPointCA->GetEntries() << " R3BGTPCProjPoint(s)";
}

void R3BGTPCLangevin::Finish() {}

ClassImp(R3BGTPCLangevin)
