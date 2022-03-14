/******************************************************************************
 *   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2019 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
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

#include "R3BGTPC.h"
#include "R3BGTPCHit2Track.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCTrackData.h"
//#include "R3BGTPCHitPar.h"

// R3BGTPCHit2Track: Constructor
R3BGTPCHit2Track::R3BGTPCHit2Track()
    : FairTask("R3B GTPC Hit to Track")
    //    , fHitParams(NULL)
    //, fHit_Par(NULL)
    , fHitCA(NULL)
    , fTrackCA(NULL)
    , fOnline(kFALSE)
{
}

R3BGTPCHit2Track::~R3BGTPCHit2Track()
{
    LOG(INFO) << "R3BGTPCHit2Track: Delete instance";
    if (fHitCA)
        delete fHitCA;
    if (fTrackCA)
        delete fTrackCA;
}

void R3BGTPCHit2Track::SetParContainers()
{
    // Reading GTPCHitPar from FairRuntimeDb
    // FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    // if (!rtdb)
    // {
    //  LOG(ERROR) << "R3BGTPCMapped2Cal:: FairRuntimeDb not opened!";
    // }

    // fHit_Par = (R3BGTPCHitPar*)rtdb->getContainer("GTPCHitPar");
    // if (!fHit_Par)
    // {
    //  LOG(ERROR) << "R3BGTPCCal2Hit::Init() Couldn't get handle on GTPCHitPar container";
    // }
    // else
    // {
    //   LOG(INFO) << "R3BGTPCCal2Hit:: GTPCHitPar container open";
    // }
}

void R3BGTPCHit2Track::SetParameter()
{
    //--- Parameter Container ---
    /*
          fHitParams = new TArrayF();
          fHitParams = fHit_Par->GetHitParams(); // Array with the Hit parameters
    */
}

InitStatus R3BGTPCHit2Track::Init()
{
    LOG(INFO) << "R3BGTPCHit2Track::Init() ";
    assert(!fTrackCA); // in case someone calls Init() twice.

    // INPUT DATA - Cal
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
        LOG(fatal) << "Init: No FairRootManager";

    fHitCA = (TClonesArray*)ioManager->GetObject("GTPCHitData");
    if (!fHitCA)
        LOG(fatal) << "Init: No GTPCHitData";

    // Register output - Track
    fTrackCA = new TClonesArray("R3BGTPCTrackData", 50);
    if (!fOnline)
    {
        ioManager->Register("GTPCTrackData", "GTPC Track", fTrackCA, kTRUE);
    }
    else
    {
        ioManager->Register("GTPCTrackData", "GTPC Track", fTrackCA, kFALSE);
    }

    SetParameter();
    return kSUCCESS;
}

InitStatus R3BGTPCHit2Track::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}

void R3BGTPCHit2Track::Exec(Option_t* opt)
{
    Reset(); // Reset entries in output arrays, local arrays

    // if (!fTrack_Par)
    // {
    //   LOG(WARNING) << "R3BGTPCHit2Track::NO Container Parameter!!";
    // }

    return;
}

void R3BGTPCHit2Track::Finish() {}

void R3BGTPCHit2Track::Reset()
{
    LOG(DEBUG) << "Clearing TrackData Structure";
    if (fTrackCA)
        fTrackCA->Clear();
}

R3BGTPCTrackData* R3BGTPCHit2Track::AddTrackData(std::size_t trackId, std::vector<R3BGTPCHitData>& hitArray)
{
    // It fills the R3BGTPCTrackData
    TClonesArray& clref = *fTrackCA;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BGTPCTrackData(trackId, std::move(hitArray));
}

ClassImp(R3BGTPCHit2Track)
