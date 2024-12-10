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

#pragma once

#include "FairTask.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCTrackData.h"
// #include "R3BGTPCHitPar.h" TrackPar?
#include "R3BGTPCTrackFinder.h"

class R3BGTPCHit2Track : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCHit2Track();

    /** Destructor **/
    ~R3BGTPCHit2Track();

    /** Virtual method Exec **/
    virtual void Exec(Option_t* opt);

    /** Virtual method Reset **/
    virtual void Reset();

    /** Virtual method SetParContainers **/
    virtual void SetParContainers();

    /** Virtual method Init **/
    virtual InitStatus Init();

    /** Virtual method ReInit **/
    virtual InitStatus ReInit();

    /** Virtual method Finish **/
    virtual void Finish();

    /** Accessor to select online mode **/
    void SetOnline(Bool_t option) { fOnline = option; }

  private:
    void SetParameter();

    // TArrayF* fHitParams;
    // or maybe
    // Double_t fHitParam;

    // R3BGTPCHitPar* fHit_Par; /**< Parameter container. >*/
    TClonesArray* fHitCA;
    TClonesArray* fTrackCA;

    Bool_t fOnline; // Selector for online data storage

    /** Private method AddTrackData**/
    //** Adds a Track to the TrackCollection
    // R3BGTPCTrackData* AddTrackData(std::size_t trackId,
    // std::vector<R3BGTPCHitData>& hitArray,std::vector<R3BGTPCHitData>*
    // hitClusterArray);

    R3BGTPCTrackFinder* fTrackFinder{};

    ClassDef(R3BGTPCHit2Track, 1);
};
