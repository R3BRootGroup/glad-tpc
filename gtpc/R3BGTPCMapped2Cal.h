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

#pragma once

#include "FairTask.h"
#include "R3BGTPCCalData.h"
#include "R3BGTPCCalPar.h"
#include "R3BGTPCMappedData.h"

class TClonesArray;
class R3BGTPCCalPar;

class R3BGTPCMapped2Cal : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCMapped2Cal();

    /** Destructor **/
    virtual ~R3BGTPCMapped2Cal();

    /** Virtual method Exec **/
    virtual void Exec(Option_t* option);

    /** Virtual method Reset **/
    virtual void Reset();

    /** Virtual method SetParContainers **/
    virtual void SetParContainers();

    // Fair specific
    /** Virtual method Init **/
    virtual InitStatus Init();

    /** Virtual method ReInit **/
    virtual InitStatus ReInit();

    /** Virtual method Finish **/
    virtual void Finish();

    /** Accessor to select online mode **/
    void SetOnline(Bool_t option) { fOnline = option; }

  private:
    void SetParameter();

    // TArrayF* fCalParams;
    // or maybe
    // Double_t fCalParam;

    R3BGTPCCalPar* fCal_Par;         /**< Parameter container. >*/
    TClonesArray* fGTPCMappedDataCA; /**< Array with GTPC Mapped- input data. >*/
    TClonesArray* fGTPCCalDataCA;    /**< Array with GTPC Cal- output data. >*/

    Bool_t fOnline; // Selector for online data storage

    /** Private method AddCalData **/
    //** Adds a GTPCCalData to the CalCollection
    R3BGTPCCalData* AddCalData(UShort_t padId, std::vector<UShort_t> adc_calib);

    ClassDef(R3BGTPCMapped2Cal, 1)
};
