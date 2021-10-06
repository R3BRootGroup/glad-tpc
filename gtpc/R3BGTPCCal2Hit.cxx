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

#include "TClonesArray.h"
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "R3BGTPC.h"
#include "R3BGTPCCalData.h"
#include "R3BGTPCHitPar.h"
#include "R3BGTPCCal2Hit.h"
#include "R3BGTPCHitData.h"

// R3BGTPCCal2Hit: Constructor
R3BGTPCCal2Hit::R3BGTPCCal2Hit()
    : FairTask("R3B GTPC Cal to Hit")
//    , fHitParams(NULL)
    , fHit_Par(NULL)
    , fCalCA(NULL)
    , fHitCA(NULL)
    , fOnline(kFALSE)
{
}

R3BGTPCCal2Hit::~R3BGTPCCal2Hit()
{
    LOG(INFO) << "R3BGTPCCal2Hit: Delete instance";
    if (fHitCA)
        delete fHitCA;
    if (fCalCA)
        delete fCalCA;
}

void R3BGTPCCal2Hit::SetParContainers()
{
  // Reading GTPCHitPar from FairRuntimeDb
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    if (!rtdb)
    {
        LOG(ERROR) << "R3BGTPCMapped2Cal:: FairRuntimeDb not opened!";
    }

    fHit_Par = (R3BGTPCHitPar*)rtdb->getContainer("GTPCHitPar");
    if (!fHit_Par)
    {
        LOG(ERROR) << "R3BGTPCCal2Hit::Init() Couldn't get handle on GTPCHitPar container";
    }
    else
    {
        LOG(INFO) << "R3BGTPCCal2Hit:: GTPCHitPar container open";
    }
}

void R3BGTPCCal2Hit::SetParameter()
{
    //--- Parameter Container ---
/*
    fHitParams = new TArrayF();
    fHitParams = fHit_Par->GetHitParams(); // Array with the Hit parameters
*/
}

InitStatus R3BGTPCCal2Hit::Init()
{
    LOG(INFO) << "R3BGTPCCal2Hit::Init() ";
    assert(!fHitCA); // in case someone calls Init() twice.

    // INPUT DATA - Cal
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
        LOG(fatal) << "Init: No FairRootManager";

    fHitCA = (TClonesArray*)ioManager->GetObject("GTPCHitData");
    if (!fHitCA)
        LOG(fatal) << "Init: No GTPCHitData";

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
    return kSUCCESS;
}

InitStatus R3BGTPCCal2Hit::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}


void R3BGTPCCal2Hit::Exec(Option_t* opt)
{
    Reset(); // Reset entries in output arrays, local arrays

    if (!fHit_Par)
    {
        LOG(WARNING) << "R3BGTPCCal2Hit::NO Container Parameter!!";
    }

    // ALGORITHMS FOR HIT FINDING
    // Nb of CrystalHits in current event
    Int_t nCals = fHitCA->GetEntries();
    if (!nCals)
        return;

    Double_t x=0,y=0,z=0,lW=0,ene=0;

    R3BGTPCCalData** calData;
    calData = new R3BGTPCCalData*[nCals];

    for (Int_t i = 0; i < nCals; i++)
    {
        calData[i] = (R3BGTPCCalData*)(fCalCA->At(i));
        UShort_t pad = calData[i]->GetPadId();
        std::vector<UShort_t> adc_cal = calData[i]->GetADC();
        //just a null hit at the moment, reconstruction should be included here
    }

    AddHitData(x,y,z,lW,ene);

    if (calData)
        delete calData;
    return;
}

void R3BGTPCCal2Hit::Finish() {}

void R3BGTPCCal2Hit::Reset()
{
    LOG(DEBUG) << "Clearing HitData Structure";
    if (fHitCA)
        fHitCA->Clear();
}


R3BGTPCHitData* R3BGTPCCal2Hit::AddHitData(Double_t x,
                       Double_t y,
                       Double_t z,
                       Double_t longWidth,
                       Double_t energy)
{
    // It fills the R3BGTPCCalData
    TClonesArray& clref = *fHitCA;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BGTPCHitData(x,y,z,longWidth,energy);
}

ClassImp(R3BGTPCCal2Hit)
