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

#include "R3BGTPCHitClusterData.h"

#include <Rtypes.h>

ClassImp(R3BGTPCHitClusterData);

R3BGTPCHitClusterData::R3BGTPCHitClusterData()
    : R3BGTPCHitData(-10000, -10000, -10000, 0.0, 0.0)
{
    fCovMatrix.ResizeTo(3, 3);
    for (Int_t iElem = 0; iElem < 9; iElem++)
        fCovMatrix(iElem / 3, iElem % 3) = 0;
}
