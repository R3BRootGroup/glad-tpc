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

#include "R3BGTPCHitData.h"

R3BGTPCHitData::R3BGTPCHitData()
    : fX(0)
    , fY(0)
    , fZ(0)
    , fLongWidth(0)
    , fEnergy(0)
{
}

R3BGTPCHitData::R3BGTPCHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy)
    : fX(x)
    , fY(y)
    , fZ(z)
    , fLongWidth(longWidth)
    , fEnergy(energy)
{
}

ClassImp(R3BGTPCHitData);
