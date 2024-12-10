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

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TClonesArray.h"

#include "R3BGTPC.h"
#include "R3BGTPCCalData.h"
#include "R3BGTPCCalPar.h"
#include "R3BGTPCMapped2Cal.h"
#include "R3BGTPCMappedData.h"

// R3BGTPCMapped2Cal: Constructor
R3BGTPCMapped2Cal::R3BGTPCMapped2Cal()
    : FairTask("R3B GTPC Calibrator")
    //    , fCalParams(NULL)
    , fCal_Par(NULL)
    , fGTPCMappedDataCA(NULL)
    , fGTPCCalDataCA(NULL)
    , fOnline(kFALSE)
{
}

R3BGTPCMapped2Cal::~R3BGTPCMapped2Cal()
{
    LOG(info) << "R3BGTPCMapped2Cal: Delete instance";
    if (fGTPCMappedDataCA)
        delete fGTPCMappedDataCA;
    if (fGTPCCalDataCA)
        delete fGTPCCalDataCA;
}

void R3BGTPCMapped2Cal::SetParContainers()
{
    // Reading GTPCCalPar from FairRuntimeDb
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    if (!rtdb)
    {
        LOG(error) << "R3BGTPCMapped2Cal:: FairRuntimeDb not opened!";
    }

    fCal_Par = (R3BGTPCCalPar*)rtdb->getContainer("GTPCCalPar");
    if (!fCal_Par)
    {
        LOG(error) << "R3BGTPCMapped2Cal::Init() Couldn't get handle on GTPCCalPar "
                      "container";
    }
    else
    {
        LOG(info) << "R3BGTPCMapped2Cal:: GTPCCalPar container open";
    }
}

void R3BGTPCMapped2Cal::SetParameter()
{
    //--- Parameter Container ---
    /*
        fCalParams = new TArrayF();
        fCalParams = fCal_Par->GetCalParams(); // Array with the Cal parameters
    */
}

InitStatus R3BGTPCMapped2Cal::Init()
{
    LOG(info) << "R3BGTPCMapped2Cal::Init()";
    assert(!fGTPCCalDataCA); // in case someone calls Init() twice.

    // INPUT DATA - Mapped
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
        LOG(fatal) << "Init: No FairRootManager";

    fGTPCMappedDataCA = (TClonesArray*)ioManager->GetObject("GTPCMappedData");
    if (!fGTPCMappedDataCA)
        LOG(fatal) << "Init: No GTPCMappedData";

    // Register OUTPUT DATA - Calibrated
    fGTPCCalDataCA = new TClonesArray("R3BGTPCCalData", 50);
    if (!fOnline)
    {
        ioManager->Register("GTPCCalData", "GTPC Cal", fGTPCCalDataCA, kTRUE);
    }
    else
    {
        ioManager->Register("GTPCCalData", "GTPC Cal", fGTPCCalDataCA, kFALSE);
    }

    SetParameter();
    return kSUCCESS;
}

InitStatus R3BGTPCMapped2Cal::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}

void R3BGTPCMapped2Cal::Exec(Option_t* option)
{
    // Reset entries in output arrays, local arrays
    Reset();

    if (!fCal_Par)
    {
        LOG(warn) << "R3BGTPCMapped2Cal::NO Container Parameter!!";
    }

    // Reading the Input -- Mapped Data --
    Int_t nHits = fGTPCMappedDataCA->GetEntries();
    if (!nHits)
        return;

    UShort_t pad = 0;
    std::vector<UShort_t> adc_cal;

    R3BGTPCMappedData** mappedData;
    mappedData = new R3BGTPCMappedData*[nHits];

    for (Int_t i = 0; i < nHits; i++)
    {
        mappedData[i] = (R3BGTPCMappedData*)(fGTPCMappedDataCA->At(i));
        pad = mappedData[i]->GetPadId();
        adc_cal = mappedData[i]->GetADC();
        // just copying, calibrating should be included here
    }
    AddCalData(pad, adc_cal);

    if (mappedData)
        delete mappedData;
    return;
}

void R3BGTPCMapped2Cal::Finish() {}

void R3BGTPCMapped2Cal::Reset()
{
    LOG(debug) << "Clearing CalData Structure";
    if (fGTPCCalDataCA)
        fGTPCCalDataCA->Clear();
}

R3BGTPCCalData* R3BGTPCMapped2Cal::AddCalData(UShort_t padId, std::vector<UShort_t> adc_calib)
{
    // It fills the R3BGTPCCalData
    TClonesArray& clref = *fGTPCCalDataCA;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BGTPCCalData(padId, adc_calib);
}

ClassImp(R3BGTPCMapped2Cal)
