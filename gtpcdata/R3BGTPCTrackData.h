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

#ifndef R3BGTPCTRACKDATA_H
#define R3BGTPCTRACKDATA_H

#include "R3BGTPCHitClusterData.h"
#include "R3BGTPCHitData.h"
#include "TObject.h"
#include <memory>
#include <stdint.h>

class R3BGTPCTrackData : public TObject
{

  public:
    // Default Constructor
    R3BGTPCTrackData();

    /** Standard Constructor

    **/
    R3BGTPCTrackData(std::size_t trackId,
                     std::vector<R3BGTPCHitData> hitArray,
                     std::vector<R3BGTPCHitClusterData> hitClusterArray);

    // Destructor
    virtual ~R3BGTPCTrackData() {}

    // Getters
    Int_t GetTrackId() { return fTrackId; }
    std::vector<R3BGTPCHitData>& GetHitArray() { return fHitArray; }
    std::vector<R3BGTPCHitClusterData>* GetHitClusterArray() { return &fHitClusterArray; }

    // Setters
    void SetTrackId(Int_t val) { fTrackId = val; }
    void AddHit(R3BGTPCHitData& hit) { fHitArray.push_back(hit); }
    void AddClusterHit(std::shared_ptr<R3BGTPCHitClusterData> hitCluster)
    {
        fHitClusterArray.push_back(std::move(*hitCluster));
    }

  protected:
    Int_t fTrackId{ -1 };                  // Track Id
    std::vector<R3BGTPCHitData> fHitArray; // Track Hit Array
    std::vector<R3BGTPCHitClusterData> fHitClusterArray;

    ClassDef(R3BGTPCTrackData, 1)
};

#endif
