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

#include "R3BGTPCHitData.h"
#include "TObject.h"
#include <stdint.h>

class R3BGTPCTrackData : public TObject
{

  public:
    // Default Constructor
    R3BGTPCTrackData();

    /** Standard Constructor

    **/
    R3BGTPCTrackData(std::size_t trackId, std::vector<R3BGTPCHitData> hitArray);

    // Destructor
    virtual ~R3BGTPCTrackData() {}

    // Getters
    inline const std::size_t GetTrackId() const { return fTrackId; }
    inline const std::vector<R3BGTPCHitData>* GetHitArray() const { return &fHitArray; }

  protected:
    std::size_t fTrackId;                  // Track Id
    std::vector<R3BGTPCHitData> fHitArray; // Track Hit Array

  public:
    void AddHit(R3BGTPCHitData &&hit) { fHitArray.push_back(hit); }

    ClassDef(R3BGTPCTrackData, 1)
};

#endif
