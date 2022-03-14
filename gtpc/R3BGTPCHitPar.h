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

#ifndef R3BGTPCHITPAR_H
#define R3BGTPCHITPAR_H

#include "FairParGenericSet.h"
#include "TObject.h"

#include "TArrayF.h"
#include "TObjArray.h"
#include <TObjString.h>

class FairParamList;

class R3BGTPCHitPar : public FairParGenericSet
{

  public:
    /** Standard constructor **/
    R3BGTPCHitPar(const char* name = "GTPCHitPar",
                  const char* title = "GTPC Hit Parameters",
                  const char* context = "GTPCHitParContext");

    /** Destructor **/
    virtual ~R3BGTPCHitPar();

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
    TArrayF* GetHitParams() { return fHitParams; }

    void SetNumParams(Int_t number) { fNumParams = number; }
    void SetHitParams(Float_t cc, Int_t ii) { fHitParams->AddAt(cc, ii); }

    /** Create more Methods if you need them! **/

  private:
    TArrayF* fHitParams; /*< Cal 2 Hit Parameters>*/
    Int_t fNumParams;    /*< number of params>*/

 
    const R3BGTPCHitPar& operator=(const R3BGTPCHitPar&);
    R3BGTPCHitPar(const R3BGTPCHitPar&);

    ClassDef(R3BGTPCHitPar, 1);
};

#endif /* R3BGTPCHITPAR_H */
