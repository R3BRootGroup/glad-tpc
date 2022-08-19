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

#include "R3BGTPCTrackData.h"

R3BGTPCTrackData::R3BGTPCTrackData()
    : fTrackId(0)
    , fHitArray(0)
{
}

R3BGTPCTrackData::R3BGTPCTrackData(std::size_t trackId, std::vector<R3BGTPCHitData> hitArray, std::vector<R3BGTPCHitClusterData> hitClusterArray)
    : fTrackId(trackId)
    , fHitArray(hitArray)
    , fHitClusterArray(hitClusterArray) 
{
}

ClassImp(R3BGTPCTrackData);
