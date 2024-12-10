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

#include "R3BGTPC.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairVolume.h"
#include "R3BGTPCPoint.h"
#include "R3BMCStack.h"
#include "TClonesArray.h"
#include "TGeoMCGeometry.h"
#include "TMCProcess.h"
#include "TObjArray.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include <iostream>
#include <stdlib.h>

// includes for modeling
#include "TGeoManager.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TVirtualMCStack.h"

#include "TGeoMedium.h"

using std::cerr;
using std::cout;
using std::endl;

#define U_MEV 931.4940954

// -----   Default constructor   -------------------------------------------
R3BGTPC::R3BGTPC()
    : R3BGTPC("")
{
}
// -------------------------------------------------------------------------

R3BGTPC::R3BGTPC(const TString& geoFile, const TGeoTranslation& trans, const TGeoRotation& rot)
    : R3BGTPC(geoFile, { trans, rot })
{
}
// -------------------------------------------------------------------------

R3BGTPC::R3BGTPC(const TString& geoFile, const TGeoCombiTrans& combi)
    : R3BDetector("R3BGTPC", kGTPC, geoFile, combi)
{
    fGTPCPointCollection = new TClonesArray("R3BGTPCPoint");
}

// -----   Destructor   ----------------------------------------------------
R3BGTPC::~R3BGTPC()
{
    if (fGTPCPointCollection)
    {
        fGTPCPointCollection->Delete();
        delete fGTPCPointCollection;
    }
}
// -------------------------------------------------------------------------

void R3BGTPC::FinishRun() { ; }

// -------------------------------------------------------------------------
void R3BGTPC::Initialize()
{
    FairDetector::Initialize();
    LOG(info) << "R3BGTPC: initialisation";
    LOG(debug) << "-I- R3BGTPC: Vol (McId) def";
    LOG(info) << "R3BGTPC: GTPC_box Vol. (McId) " << gMC->VolId("GTPC_box");
    LOG(info) << "R3BGTPC: Active_region Vol. (McId) " << gMC->VolId("Active_region");
}

//____________________________________________________________
void R3BGTPC::SetSpecialPhysicsCuts()
{
    LOG(info) << "-I- R3BGTPC: Adding customized Physics cut ... ";

    if (gGeoManager)
    { // NOT SURE THIS IS USEFUL
        TGeoMedium* pmix = gGeoManager->GetMedium("P10");
        if (pmix)
        {
            // Setting processes for Gas
            gMC->Gstpar(pmix->GetId(), "LOSS", 3);
            gMC->Gstpar(pmix->GetId(), "STRA", 1.0);
            gMC->Gstpar(pmix->GetId(), "PAIR", 1.0);
            gMC->Gstpar(pmix->GetId(), "COMP", 1.0);
            gMC->Gstpar(pmix->GetId(), "PHOT", 1.0);
            gMC->Gstpar(pmix->GetId(), "ANNI", 1.0);
            gMC->Gstpar(pmix->GetId(), "BREM", 1.0);
            gMC->Gstpar(pmix->GetId(), "HADR", 1.0);
            gMC->Gstpar(pmix->GetId(), "DRAY", 1.0);
            gMC->Gstpar(pmix->GetId(), "DCAY", 1.0);
            gMC->Gstpar(pmix->GetId(), "MULS", 1.0);
            gMC->Gstpar(pmix->GetId(), "RAYL", 1.0);

            // Setting Energy-CutOff for Gas Only->Ionization energy ~30eV
            Double_t cutE = 1e-6; // GeV-> 1keV

            LOG(info) << "-I- R3BGTPC: Medium Id " << pmix->GetId() << " Energy Cut-Off : " << cutE << " GeV";

            gMC->Gstpar(pmix->GetId(), "CUTGAM", cutE); /** gammas (GeV)*/
            gMC->Gstpar(pmix->GetId(), "CUTELE", cutE); /** electrons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "CUTNEU", cutE); /** neutral hadrons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "CUTHAD", cutE); /** charged hadrons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "CUTMUO", cutE); /** muons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "BCUTE", cutE);  /** electron bremsstrahlung (GeV)*/
            gMC->Gstpar(pmix->GetId(), "BCUTM", cutE);  /** muon and hadron bremsstrahlung(GeV)*/
            gMC->Gstpar(pmix->GetId(), "DCUTE", cutE);  /** delta-rays by electrons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "DCUTM", cutE);  /** delta-rays by muons (GeV)*/
            gMC->Gstpar(pmix->GetId(), "PPCUTM", -1.);  /** direct pair production by muons (GeV)*/
        }
    } //! gGeoManager
}

// -----   Public method ProcessHits  --------------------------------------
Bool_t R3BGTPC::ProcessHits(FairVolume* vol)
{
    TLorentzVector pos;
    TLorentzVector mom;
    gMC->TrackPosition(pos);
    gMC->TrackMomentum(mom);
    Int_t theTrackStatus = GetTrackStatus(gMC->IsNewTrack(),
                                          gMC->IsTrackDisappeared(),
                                          gMC->IsTrackStop(),
                                          gMC->IsTrackAlive(),
                                          gMC->IsTrackEntering(),
                                          gMC->IsTrackExiting(),
                                          gMC->IsTrackInside(),
                                          gMC->IsTrackOut());

    Int_t parentTrackID = gMC->GetStack()->GetCurrentParentTrackNumber();
    TString particleName = gMC->GetStack()->GetCurrentTrack()->GetName();
    //_______________only care about primary particle
    // if (parentTrackID == -1 || (parentTrackID == 0 && particleName != "e-"))
    if (gMC->TrackPid() != 0) // due to the INCL generator
    {
        Int_t size = fGTPCPointCollection->GetEntriesFast();
        new ((*fGTPCPointCollection)[size])
            R3BGTPCPoint(gMC->GetStack()->GetCurrentTrackNumber(), // trackID
                         vol->getModId(),                          // check if getModId or CurrentVolOffID(1,modID)
                         pos.Vect(),                               // pos from gMC->TrackPosition(pos);
                         mom.Vect(),                               // mom from gMC->TrackMomentum(pos);
                         gMC->TrackTime(),                         // time in ns
                         gMC->TrackLength(),                       // Return the length of the current track from its
                                                                   // origin (in cm)
                         gMC->Edep(),                              // eloss
                         gMC->CurrentEvent(),                      // EventID
                         parentTrackID,                            // parentTrackID
                         gMC->GetStack()->GetCurrentTrack()->GetMother(0), // primaryParticleID
                         theTrackStatus,                                   // trackStatus from GetTrackStatus(...)
                         gMC->TrackPid(),                                  // PDGCode
                         vol->getModId(),                                  // moduleID
                         vol->getCopyNo(),                                 // detCopyID
                         particleName,                                     // particleName
                         vol->GetName(),                                   // volName (or vol->getRealName();??)
                         TMCProcessName[gMC->ProdProcess(0)],              // processName
                         gMC->TrackCharge(),                               // charge
                         gMC->TrackMass(),                 // Return the mass of the track currently transported.
                         (gMC->Etot() - gMC->TrackMass()), // kineticEnergy
                         gMC->TrackStep(),                 // Return the length in centimeters of the current
                                                           // step (in cm)
                         kTRUE);                           // isAccepted
    }

    // Increment number of LandPoints for this track
    // R3BStack* stack = (R3BStack*)gMC->GetStack();
    // stack->AddPoint(kGTPC);

    return kTRUE;
}

// ----    Public method BeginOfEvent -----------------------------------------
void R3BGTPC::BeginEvent() { ; }

// -----   Public method EndOfEvent   -----------------------------------------
void R3BGTPC::EndOfEvent()
{
    if (fVerboseLevel)
        Print();
    Reset();
}
// ----------------------------------------------------------------------------

// -----   Public method Register   -------------------------------------------
void R3BGTPC::Register() { FairRootManager::Instance()->Register("GTPCPoint", GetName(), fGTPCPointCollection, kTRUE); }
// ----------------------------------------------------------------------------

// -----   Public method GetCollection   --------------------------------------
TClonesArray* R3BGTPC::GetCollection(Int_t iColl) const
{
    if (iColl == 0)
    {
        return fGTPCPointCollection;
    }
    return nullptr;
}
// ----------------------------------------------------------------------------

// -----   Public method Print   ----------------------------------------------
void R3BGTPC::Print(Option_t* option) const
{
    Int_t nhits = fGTPCPointCollection->GetEntriesFast();
    LOG(info) << "R3BGTPC: " << nhits << " points registered in this event";
}
// ----------------------------------------------------------------------------

// -----   Public method Reset   ----------------------------------------------
void R3BGTPC::Reset() { fGTPCPointCollection->Clear(); }

//_________________________________________________________
Bool_t R3BGTPC::CheckIfSensitive(std::string name)
{
    LOG(info) << "R3BGTPC::CheckIfSensitive " << name;

    if (TString(name).Contains("Active_region"))
    {
        LOG(info) << name << " is sensitive";
        return kTRUE;
    }
    return kFALSE;
}

Int_t R3BGTPC::GetTrackStatus(bool NewTrack,
                              bool TrackDisappeared,
                              bool TrackStop,
                              bool TrackAlive,
                              bool TrackEntering,
                              bool TrackExiting,
                              bool TrackInside,
                              bool TrackOut)
{
    int trackstatus = 1 * TrackOut + 10 * TrackInside + 100 * TrackExiting + 1000 * TrackEntering + 10000 * TrackAlive +
                      100000 * TrackStop + 1000000 * TrackDisappeared + 10000000 * NewTrack;

    return trackstatus;
}

ClassImp(R3BGTPC)
