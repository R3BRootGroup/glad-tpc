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

#include "FairLogger.h"
#include "FairParGenericSet.h"
#include "TMath.h"
#include "TObject.h"

using namespace std;

class FairParamList;

class R3BGTPCGasPar : public FairParGenericSet
{

  public:
    R3BGTPCGasPar(const char* name = "GTPCGasPar",
                  const char* title = "GTPC Gas Parameters",
                  const char* context = "GTPCGasParContext");

    ~R3BGTPCGasPar();

    /** Reset all parameters **/
    virtual void clear();

    /** Store all parameters using FairRuntimeDB **/
    virtual void putParams(FairParamList* list);

    /** Retrieve all parameters using FairRuntimeDB**/
    Bool_t getParams(FairParamList* list);

    /** Print values of parameters to the standard output **/
    virtual void print();
    void printParams();

    /** Accessor functions **/
    const Double_t GetLongDiff() { return LongDiff; }
    const Double_t GetTransDiff() { return TransDiff; }
    const Double_t GetFanoFactor() { return FanoFactor; }
    const Double_t GetEIonization() { return EIonization; }
    const Double_t GetDriftVelocity() { return DriftVelocity; }

    void SetLongDiff(Double_t value) { LongDiff = value; }
    void SetTransDiff(Double_t value) { TransDiff = value; }
    void SetFanoFactor(Double_t value) { FanoFactor = value; }
    void SetEIonization(Double_t value) { EIonization = value; }
    void SetDriftVelocity(Double_t value) { DriftVelocity = value; }

  private:
    Double_t LongDiff, TransDiff; // [cm^2/ns]
    Double_t FanoFactor;
    Double_t EIonization;   // [GeV]
    Double_t DriftVelocity; // [cm/ns]

    const R3BGTPCGasPar& operator=(const R3BGTPCGasPar&); /*< an assignment operator>*/
    R3BGTPCGasPar(const R3BGTPCGasPar&);                  /*< a copy constructor >*/

    ClassDef(R3BGTPCGasPar, 1)
};
