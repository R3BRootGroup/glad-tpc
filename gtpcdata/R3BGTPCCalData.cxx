/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "R3BGTPCCalData.h"

R3BGTPCCalData::R3BGTPCCalData()
    : fPadId(0)
    , fADC(0)
{
}

R3BGTPCCalData::R3BGTPCCalData(UShort_t padId, std::vector<UShort_t> adc)
    : fPadId(padId)
    , fADC(adc)
{
}

ClassImp(R3BGTPCCalData);
