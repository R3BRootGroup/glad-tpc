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

class R3BGTPCElecPar : public FairParGenericSet
{

  public:
    R3BGTPCElecPar(const char* name = "GTPCElecPar",
                   const char* title = "GTPC Elecronic Parameters",
                   const char* context = "GTPCElecParContext");

    ~R3BGTPCElecPar();

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
    const Double_t GetGain() { return Gain; }
    const Double_t GetTheta() { return Theta; }
    const Double_t GetNoiseRMS() { return NoiseRMS; }
    const Double_t GetTimeBinSize() { return TimeBinSize; }
    const Double_t GetShapingTime() { return ShapingTime; }
    const Double_t GetThreshold() { return Threshold; }
    const Double_t GetDriftEField() { return DriftEField; }
    const Double_t GetDriftTimeStep() { return DriftTimeStep; }

    void SetGain(Double_t value) { Gain = value; }
    void SetTheta(Double_t value) { Theta = value; }
    void SetNoiseRMS(Double_t value) { NoiseRMS = value; }
    void SetTimeBinSize(Double_t value) { TimeBinSize = value; }
    void SetShapingTime(Double_t value) { ShapingTime = value; }
    void SetThreshold(Double_t value) { Threshold = value; }
    void SetDriftEField(Double_t value) { DriftEField = value; }
    void SetDriftTimeStep(Double_t value) { DriftTimeStep = value; }

  private:
    Double_t Gain;          //
    Double_t Theta;         //
    Double_t NoiseRMS;      //
    Double_t TimeBinSize;   // [ns]
    Double_t ShapingTime;   //
    Double_t Threshold;     //
    Double_t DriftEField;   // [V/cm]
    Double_t DriftTimeStep; // [ns]

    const R3BGTPCElecPar& operator=(const R3BGTPCElecPar&); /*< an assignment operator>*/
    R3BGTPCElecPar(const R3BGTPCElecPar&);                  /*< a copy constructor >*/

    ClassDef(R3BGTPCElecPar, 1)
};
