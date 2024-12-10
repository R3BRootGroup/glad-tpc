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

#include "FairParGenericSet.h"
#include "TObject.h"

#include "TArrayF.h"
#include "TObjArray.h"
#include <TObjString.h>

class FairParamList;

class R3BGTPCCalPar : public FairParGenericSet
{

  public:
    /** Standard constructor **/
    R3BGTPCCalPar(const char* name = "GTPCCalPar",
                  const char* title = "GTPC Cal Parameters",
                  const char* context = "GTPCCalParContext");

    /** Destructor **/
    virtual ~R3BGTPCCalPar();

    /** Method to reset all parameters **/
    virtual void clear();

    /** Method to store all parameters using FairRuntimeDB **/
    virtual void putParams(FairParamList* list);

    /** Method to retrieve all parameters using FairRuntimeDB**/
    Bool_t getParams(FairParamList* list);

    /** Method to print values of parameters to the standard output **/
    virtual void print();
    void printParams();

    /** Accessor functions **/
    const Double_t GetNumParams() { return fNumParams; }
    TArrayF* GetCalParams() { return fCalParams; }

    void SetNumParams(Int_t number) { fNumParams = number; }
    void SetCalParams(Float_t cc, Int_t ii) { fCalParams->AddAt(cc, ii); }

    /** Create more Methods if you need them! **/

  private:
    TArrayF* fCalParams; /*< Calibration Parameters>*/
    Int_t fNumParams;    /*< number of params>*/

    const R3BGTPCCalPar& operator=(const R3BGTPCCalPar&);
    R3BGTPCCalPar(const R3BGTPCCalPar&);

    ClassDef(R3BGTPCCalPar, 1);
};
