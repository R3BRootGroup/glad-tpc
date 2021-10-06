/******************************************************************************
 *   Copyright (C) 2020 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2020 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "R3BGTPCGasPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ----------------------------------------------
R3BGTPCGasPar::R3BGTPCGasPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
{
}

// ----  Destructor -------------------------------------------------------
R3BGTPCGasPar::~R3BGTPCGasPar() { clear(); }

// ----  Method clear ----------------------------------------------------------
void R3BGTPCGasPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BGTPCGasPar::putParams(FairParamList* list)
{
    LOG(INFO) << "R3BGTPCGasPar::putParams() called";
    if (!list)
    {
        return;
    }
    list->add("GTPCLongDiff", LongDiff);
    list->add("GTPCTransDiff", TransDiff);
    list->add("GTPCFanoFactor", FanoFactor);
    list->add("GTPCEIonization", EIonization);
    list->add("GTPCDriftVelocity", DriftVelocity);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCGasPar::getParams(FairParamList* list)
{
    LOG(INFO) << "R3BCalifR3BGTPCGasParaMappingPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }
    if (!(list->fill("GTPCLongDiff", &LongDiff)))
    {
        LOG(INFO) << "---Could not initialize GTPCLongDiff";
        return kFALSE;
    }
    if (!(list->fill("GTPCTransDiff", &TransDiff)))
    {
        LOG(INFO) << "---Could not initialize GTPCTransDiff";
        return kFALSE;
    }
    if (!(list->fill("GTPCFanoFactor", &FanoFactor)))
    {
        LOG(INFO) << "---Could not initialize GTPCFanoFactor";
        return kFALSE;
    }
    if (!(list->fill("GTPCEIonization", &EIonization)))
    {
        LOG(INFO) << "---Could not initialize GTPCEIonization";
        return kFALSE;
    }
    if (!(list->fill("GTPCDriftVelocity", &DriftVelocity)))
    {
        LOG(INFO) << "---Could not initialize GTPCDriftVelocity";
        return kFALSE;
    }
    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCGasPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCGasPar::printParams()
{
    LOG(INFO) << "R3BGTPCGasPar: GTPC Gas Parameters:";

    LOG(INFO) << "GTPCLongDiff " << LongDiff << " cm^2/ns,  "
              << "GTPCTransDiff " << TransDiff << " cm^2/ns" << endl;
    LOG(INFO) << "GTPCFanoFactor " << FanoFactor << ",  "
              << "GTPCEIonization " << EIonization << " GeV,  "
              << "GTPCDriftVelocity " << DriftVelocity << " cm/ns,  " << endl;
}

ClassImp(R3BGTPCGasPar);
