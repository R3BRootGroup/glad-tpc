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
#include "R3BGTPCProjector.h"
#include "R3BMCTrack.h"
#include "TClonesArray.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "R3BGTPCSetup.h"
#include "TF1.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

std::string geotag = "Prototype";
std::string geotag1 = "FullBeamOut";
std::string geotag2 = "FullBeamIn";
std::string GEOTAG = geotag; 				//For the moment the projection is done only for the Prototype
R3BGTPCProjector::R3BGTPCProjector()
    : FairTask("R3BGTPCProjector")
{
    R3BGTPCSetup* setup = new R3BGTPCSetup(GEOTAG, 1);
    LOG(INFO) << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG;
    fEIonization = setup->GetEIonization();     // [GeV]
    fDriftVelocity = setup->GetDriftVelocity(); // [cm/ns]
    fTransDiff = setup->GetTransDiff();         // [cm^2/ns]
    fLongDiff = setup->GetLongDiff();           // [cm^2/ns]
    fFanoFactor = setup->GetFanoFactor();
    fSizeOfVirtualPad = setup->GetPadSize(); // 5 means: pads of 2*2 mm^2
    fHalfSizeTPC_X = 0.;                     // to be filled during Init()
    fHalfSizeTPC_Y = 0.;                     // to be filled during Init()
    fHalfSizeTPC_Z = 0.;                     // to be filled during Init()
    delete setup;
}

R3BGTPCProjector::~R3BGTPCProjector()
{
    fGTPCPoints->Clear();
    MCTrackCA->Clear();
    fGTPCProjPoint->Clear();
}

void R3BGTPCProjector::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    if (!run)
    {
        LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No analysis run";
        return;
    }
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (!rtdb)
    {
        LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No runtime database";
        return;
    }
}

InitStatus R3BGTPCProjector::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
    {
        LOG(FATAL) << "R3BGTPCProjector::Init: No FairRootManager";
        return kFATAL;
    }
    // Input: TClonesArray of R3BGTPCPoints
    if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr)
    {
        LOG(FATAL) << "R3BGTPCProjector::Init No GTPCPoint!";
        return kFATAL;
    }
    fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");
		// Input: TClonesArray of R3BMCTrack
    if ((TClonesArray*)ioman->GetObject("MCTrack") == nullptr)
    {
        LOG(FATAL) << "R3BMCTrack::Init No MCTrack!";
        return kFATAL;
    }    
    MCTrackCA = (TClonesArray*)ioman->GetObject("MCTrack");

    // Output: TClonesArray of R3BGTPCProjPoint
    fGTPCProjPoint = new TClonesArray("R3BGTPCProjPoint");
    ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);

    R3BGTPCSetup* setup = new R3BGTPCSetup(GEOTAG, 1);
    fHalfSizeTPC_X = setup->GetActiveRegionx() / 2.;
    fHalfSizeTPC_Y = setup->GetActiveRegiony() / 2.;
    fHalfSizeTPC_Z = setup->GetActiveRegionz() / 2.;
    delete setup;
    return kSUCCESS;
}

void R3BGTPCProjector::SetDriftParameters(Double_t ion,
                                          Double_t driftv,
                                          Double_t tDiff,
                                          Double_t lDiff,
                                          Double_t fanoFactor)
{
    fEIonization = ion;      // [GeV]
    fDriftVelocity = driftv; // [cm/ns]
    fTransDiff = tDiff;      // [cm^(-1/2)]
    fLongDiff = lDiff;       // [cm^(-1/2)]
    fFanoFactor = fanoFactor;
}

void R3BGTPCProjector::SetSizeOfVirtualPad(Double_t size)
{
    fSizeOfVirtualPad = size; // 1 means pads of 1cm^2, 5 means pads of 2mm^2, ...
}

void R3BGTPCProjector::Exec(Option_t*)
{

    fGTPCProjPoint->Clear("C");

    Int_t nPoints = fGTPCPoints->GetEntries();
    LOG(INFO) << "R3BGTPCProjector: processing " << nPoints << " points";
    if (nPoints < 2)
    {
        LOG(INFO) << "Not enough hits for digitization! (<2)";
        return;
    }

    R3BGTPCPoint* aPoint;
    Int_t presentTrackID = -10; // control of the point trackID
    Double_t xPre, yPre, zPre;
    Double_t xPost, yPost, zPost;
    Double_t projX, projZ, projTime;
    Double_t energyDep = 0.;
    Double_t timeBeforeDrift = 0.;
    Bool_t readyToProject = kFALSE;
    Bool_t virtualPadFound = kFALSE;
    Int_t electrons = 0;
    Int_t flucElectrons = 0;
    Int_t generatedElectrons = 0;
    Double_t stepX, stepY, stepZ;
    Double_t driftDistance, driftTime;
    Double_t sigmaLongAtPadPlane;
    Double_t sigmaTransvAtPadPlane;
    Int_t evtID;
    for (Int_t i = 0; i < nPoints; i++)
    {
        aPoint = (R3BGTPCPoint*)fGTPCPoints->At(i);
        evtID = aPoint->GetEventID();
        Int_t PDGCode, MotherId;
        Double_t Vertex_x0, Vertex_y0, Vertex_z0, Vertex_px0, Vertex_py0, Vertex_pz0;
        if (aPoint->GetTrackStatus() == 11000 || aPoint->GetTrackStatus() == 10010010 ||
            aPoint->GetTrackStatus() == 10010000 || aPoint->GetTrackStatus() == 10011000)
        {
            // entering the gas volume or new track inside the gas (is 10010010 or 10010000??)
            presentTrackID = aPoint->GetTrackID();
            xPre = aPoint->GetX();
            yPre = aPoint->GetY();
            zPre = aPoint->GetZ();
            R3BMCTrack* Track=(R3BMCTrack*)MCTrackCA->At(presentTrackID);
            PDGCode=Track->GetPdgCode();
            MotherId=Track->GetMotherId();
            Vertex_x0=Track->GetStartX();
            Vertex_y0=Track->GetStartY();
            Vertex_z0=Track->GetStartZ();
            Vertex_px0=Track->GetPx();
            Vertex_py0=Track->GetPy();
            Vertex_pz0=Track->GetPz();
            readyToProject = kTRUE;
            continue; // no energy deposited in this point, just taking in entrance coordinates
        }
        else
        { // any other case
            if (presentTrackID != aPoint->GetTrackID())
            {
                cout << aPoint->GetTrackStatus() << "  " << endl;
                LOG(FATAL) << "R3BGTPCProjector::Exec: Problem 2 in point logic";
                break;
            }
            if (readyToProject != kTRUE)
            {
                LOG(FATAL) << "R3BGTPCProjector::Exec: Problem 3 in point logic";
                break;
            }
            if (aPoint->GetTrackStatus() == 10100 || aPoint->GetTrackStatus() == 1000000)
            { // exiting the gas volume or dissappearing
                readyToProject = kFALSE;
            }

            xPost = aPoint->GetX();
            yPost = aPoint->GetY();
            zPost = aPoint->GetZ();
            energyDep = aPoint->GetEnergyLoss();
            timeBeforeDrift = aPoint->GetTime();
        }
        // primary electrons produced by the ionization
        electrons = energyDep / fEIonization;
        // electron number fluctuates as the square root of the
        // Fano factor times the number of electrons
        flucElectrons = pow(fFanoFactor * electrons, 0.5);
        generatedElectrons = gRandom->Gaus(electrons, flucElectrons); // generated electrons

        // step in each direction for an homogeneous electron creation position along the track
        stepX = (xPost - xPre) / generatedElectrons;
        stepY = (yPost - yPre) / generatedElectrons;
        stepZ = (zPost - zPre) / generatedElectrons;

        // taken a mean driftDistance for the calculation of the sigmaLong and sigmaTrans
        // improve (make the calculation individual for electron) if needed, but probably slower
        Double_t yApprox = (yPost + yPre) / 2;
        driftDistance = yApprox + fHalfSizeTPC_Y;
        // cout<<"DriftDistance="<<driftDistance<<"	yApprox="<<yApprox<<endl;
        sigmaLongAtPadPlane = sqrt(driftDistance * 2 * fLongDiff / fDriftVelocity);
        sigmaTransvAtPadPlane = sqrt(driftDistance * 2 * fTransDiff / fDriftVelocity);

        for (Int_t ele = 1; ele <= generatedElectrons; ele++) // following each electrons from production to pad
        {
            driftTime = ((yPre + stepY * ele) + fHalfSizeTPC_Y) / fDriftVelocity;
            projX = gRandom->Gaus(xPre + stepX * ele, sigmaTransvAtPadPlane);
            projZ = gRandom->Gaus(zPre + stepZ * ele, sigmaTransvAtPadPlane);
            projTime = gRandom->Gaus(driftTime + timeBeforeDrift, sigmaLongAtPadPlane / fDriftVelocity);
            // cout<<"projTime="<<projTime<<"		driftTime="<<driftTime<<"
            // timeBeforeDrift="<<timeBeforeDrift<<endl;	cout<<"ProjZ="<<projZ<<" ProjX="<<projX<<endl; 
            // obtain padID for projX, projZ (simple algorithm) for the Prototype
            // the algorithm assigns a pad number which depends on the projX and projZ,
            // taking into consideration the Offset (that depends on the position inside GLAD),
            // the X and Z where the pad plane starts.
            // Since the prototype pad plane has 44x126 pads, to avoid repetition in the padID 
            // Z is multiplied by 45 and since the size of the pad is 2x2 mm^2, X and Z are divided by 0.2 cm
            // ZOffset- z-> the first pad row in the laboratory frame
            double ZOffset = 272.7; 
            // XOffset-x-> the first pad column in the laboratory frame
            double XOffset = 5.8; 
            if (projZ < ZOffset)
                projZ = ZOffset;
            if (projZ > ZOffset + 2 * fHalfSizeTPC_Z)
                projZ = ZOffset + 2 * fHalfSizeTPC_Z;
            if (projX < XOffset)
                projX = XOffset;
            if (projX > XOffset + 2 * fHalfSizeTPC_X)
                projX = XOffset + 2 * fHalfSizeTPC_X;
            Int_t padID;
            if (GEOTAG.compare("Prototype") == 0)
                padID = (44) * (Int_t)((projZ - ZOffset) / 0.2) + (Int_t)((projX - XOffset) / 0.2); // 2mm
            else
                padID = (2 * fHalfSizeTPC_X * fSizeOfVirtualPad) * (Int_t)((projZ - ZOffset) * fSizeOfVirtualPad) +
                        (Int_t)((projX - XOffset) * fSizeOfVirtualPad); //FULL HYDRA padplane has not been decided yet

            Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();
            for (Int_t pp = 0; pp < nProjPoints; pp++)
            {
                if (((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->GetVirtualPadID() == padID)
                {
                    // already existing R3BGTPCProjPoint... add time and electron
                    ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->AddCharge();											 //
                    ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->SetTimeDistr(projTime / 1000, 1); // micros
                    virtualPadFound = kTRUE;
                    break;
                }
            }
            if (!virtualPadFound)
            {
                new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID, 
                																											projTime/1000, //micros
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
            virtualPadFound = kFALSE;
        }
        xPre = xPost;
        yPre = yPost;
        zPre = zPost;
    }
    LOG(INFO) << "R3BGTPCProjector: produced " << fGTPCProjPoint->GetEntries() << " projPoints";
}

void R3BGTPCProjector::Finish() {}

ClassImp(R3BGTPCProjector)
