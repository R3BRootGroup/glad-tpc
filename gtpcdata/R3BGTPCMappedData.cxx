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

#include "R3BGTPCMappedData.h"

R3BGTPCMappedData::R3BGTPCMappedData()
    : fPadId(0)
    , fADC(0)
    , fIsValid(0)
    , fIsPedestalSubtracted(0)
{
}

R3BGTPCMappedData::R3BGTPCMappedData(UShort_t padId,
                                     std::vector<UShort_t> adc,
                                     Bool_t isValid,
                                     Bool_t isPedestalSubtracted)
    : fPadId(padId)
    , fADC(adc)
    , fIsValid(isValid)
    , fIsPedestalSubtracted(isPedestalSubtracted)
{
}

ClassImp(R3BGTPCMappedData);
