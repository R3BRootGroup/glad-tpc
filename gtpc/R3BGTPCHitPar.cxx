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

#include "R3BGTPCHitPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ---------------------------------------------------
R3BGTPCHitPar::R3BGTPCHitPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
    , fNumParams(1)
{
    fHitParams = new TArrayF(fNumParams);
}

// ----  Destructor ------------------------------------------------------------
R3BGTPCHitPar::~R3BGTPCHitPar()
{
    clear();
    if (fHitParams)
        delete fHitParams;
}

// ----  Method clear ----------------------------------------------------------
void R3BGTPCHitPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BGTPCHitPar::putParams(FairParamList* list)
{
    LOG(INFO) << "R3BGTPCHitPar::putParams() called";
    if (!list)
    {
        return;
    }

    fHitParams->Set(fNumParams);

    list->add("GTPCHitPar", *fHitParams);
    list->add("GTPCNumberPars", fNumParams);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCHitPar::getParams(FairParamList* list)
{
    LOG(INFO) << "R3BGTPCHitPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }

    if (!list->fill("GTPCNumberPars", &fNumParams))
    {
        return kFALSE;
    }

    fHitParams->Set(fNumParams);

    if (!(list->fill("GTPCHitPar", fHitParams)))
    {
        LOG(INFO) << "---Could not initialize GTPCHitPar";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCHitPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCHitPar::printParams()
{
    LOG(INFO) << "R3BGTPCHitPar::GTPC  Calibration Parameters: ";
    for (Int_t i = 0; i < fNumParams; i++)
    {
        LOG(INFO) << "HitParam(" << i << ") = " << fHitParams->GetAt(i);
    }
}

ClassImp(R3BGTPCHitPar)
