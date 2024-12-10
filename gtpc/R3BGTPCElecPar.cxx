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
    LOG(info) << "R3BGTPCElecPar::putParams() called";
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
    list->add("GTPCDriftEField", DriftEField);
    list->add("GTPCDriftTimeStep", DriftTimeStep);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCElecPar::getParams(FairParamList* list)
{
    LOG(info) << "R3BCalifR3BGTPCElecParaMappingPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }
    if (!list->fill("GTPCGain", &Gain))
    {
        LOG(info) << "---Could not initialize GTPCGain";
        return kFALSE;
    }
    if (!(list->fill("GTPCTheta", &Theta)))
    {
        LOG(info) << "---Could not initialize GTPCTheta";
        return kFALSE;
    }
    if (!(list->fill("GTPCNoiseRMS", &NoiseRMS)))
    {
        LOG(info) << "---Could not initialize GTPCNoiseRMS";
        return kFALSE;
    }
    if (!(list->fill("GTPCTimeBinSize", &TimeBinSize)))
    {
        LOG(info) << "---Could not initialize GTPCTimeBinSize";
        return kFALSE;
    }
    if (!(list->fill("GTPCShapingTime", &ShapingTime)))
    {
        LOG(info) << "---Could not initialize GTPCShapingTime";
        return kFALSE;
    }
    if (!(list->fill("GTPCThreshold", &Threshold)))
    {
        LOG(info) << "---Could not initialize GTPCThreshold";
        return kFALSE;
    }
    if (!(list->fill("GTPCDriftEField", &DriftEField)))
    {
        LOG(info) << "---Could not initialize GTPCDriftEField";
        return kFALSE;
    }
    if (!(list->fill("GTPCDriftTimeStep", &DriftTimeStep)))
    {
        LOG(info) << "---Could not initialize GTPCDriftTimeStep";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCElecPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCElecPar::printParams()
{
    LOG(info) << "R3BGTPCElecPar: GTPC Elec Parameters:";

    LOG(info) << "GTPCGain " << Gain << ",  "
              << "GTPCTheta (Polya) " << Theta << ",  "
              << "GTPCNoiseRMS " << NoiseRMS << " e- RMS,  "
              << "GTPCTimeBinSize " << TimeBinSize << " ns,  "
              << "GTPCShapingTime " << ShapingTime << " ns, "
              << "GTPCThreshold " << Threshold << " times noise rms, "
              << "GTPCDriftEField " << DriftEField << " V/cm, "
              << "GTPCDriftTimeStep " << DriftTimeStep << " ns" << endl;
}

ClassImp(R3BGTPCElecPar)
