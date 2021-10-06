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

#include "R3BGTPCElecPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ----------------------------------------------
R3BGTPCElecPar::R3BGTPCElecPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
{
}

// ----  Destructor -------------------------------------------------------
R3BGTPCElecPar::~R3BGTPCElecPar() { clear(); }

// ----  Method clear ----------------------------------------------------------
void R3BGTPCElecPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BGTPCElecPar::putParams(FairParamList* list)
{
    LOG(INFO) << "R3BGTPCElecPar::putParams() called";
    if (!list)
    {
        return;
    }
    list->add("GTPCGain", Gain);
    list->add("GTPCTheta", Theta);
    list->add("GTPCNoiseRMS", NoiseRMS);
    list->add("GTPCTimeBinSize", TimeBinSize);
    list->add("GTPCShapingTime", ShapingTime);
    list->add("GTPCThreshold", Threshold);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCElecPar::getParams(FairParamList* list)
{
    LOG(INFO) << "R3BCalifR3BGTPCElecParaMappingPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }
    if (!list->fill("GTPCGain", &Gain))
    {
        LOG(INFO) << "---Could not initialize GTPCGain";
        return kFALSE;
    }
    if (!(list->fill("GTPCTheta", &Theta)))
    {
        LOG(INFO) << "---Could not initialize GTPCTheta";
        return kFALSE;
    }
    if (!(list->fill("GTPCNoiseRMS", &NoiseRMS)))
    {
        LOG(INFO) << "---Could not initialize GTPCNoiseRMS";
        return kFALSE;
    }
    if (!(list->fill("GTPCTimeBinSize", &TimeBinSize)))
    {
        LOG(INFO) << "---Could not initialize GTPCTimeBinSize";
        return kFALSE;
    }
    if (!(list->fill("GTPCShapingTime", &ShapingTime)))
    {
        LOG(INFO) << "---Could not initialize GTPCShapingTime";
        return kFALSE;
    }
    if (!(list->fill("GTPCThreshold", &Threshold)))
    {
        LOG(INFO) << "---Could not initialize GTPCThreshold";
        return kFALSE;
    }
    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCElecPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCElecPar::printParams()
{
    LOG(INFO) << "R3BGTPCElecPar: GTPC Elec Parameters:";

    LOG(INFO) << "GTPCGain " << Gain << ",  "
              << "GTPCTheta (Polya) " << Theta << ",  "
              << "GTPCNoiseRMS " << NoiseRMS << " e- RMS,  "
              << "GTPCTimeBinSize " << TimeBinSize << " ns,  "
              << "GTPCShapingTime " << ShapingTime << " ns"
              << "GTPCThreshold " << Threshold << " times noise rms" << endl;
}

ClassImp(R3BGTPCElecPar);
