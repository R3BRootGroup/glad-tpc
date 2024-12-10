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

#include "R3BGTPCCalPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ---------------------------------------------------
R3BGTPCCalPar::R3BGTPCCalPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
    , fNumParams(1)
{
    fCalParams = new TArrayF(fNumParams);
}

// ----  Destructor ------------------------------------------------------------
R3BGTPCCalPar::~R3BGTPCCalPar()
{
    clear();
    if (fCalParams)
        delete fCalParams;
}

// ----  Method clear ----------------------------------------------------------
void R3BGTPCCalPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BGTPCCalPar::putParams(FairParamList* list)
{
    LOG(info) << "R3BGTPCCalPar::putParams() called";
    if (!list)
    {
        return;
    }

    fCalParams->Set(fNumParams);

    list->add("GTPCCalPar", *fCalParams);
    list->add("GTPCNumberPars", fNumParams);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCCalPar::getParams(FairParamList* list)
{
    LOG(info) << "R3BGTPCCalPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }

    if (!list->fill("GTPCNumberPars", &fNumParams))
    {
        return kFALSE;
    }

    fCalParams->Set(fNumParams);

    if (!(list->fill("GTPCCalPar", fCalParams)))
    {
        LOG(info) << "---Could not initialize GTPCCalPar";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCCalPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCCalPar::printParams()
{
    LOG(info) << "R3BGTPCCalPar::GTPC  Calibration Parameters: ";
    for (Int_t i = 0; i < fNumParams; i++)
    {
        LOG(info) << "calParam(" << i << ") = " << fCalParams->GetAt(i);
    }
}

ClassImp(R3BGTPCCalPar)
