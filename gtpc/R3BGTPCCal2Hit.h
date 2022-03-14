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

#ifndef R3BGTPCCAL2HIT_H
#define R3BGTPCCAL2HIT_H

#include "FairTask.h"
#include "R3BGTPCCalData.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCHitPar.h"
#include "R3BGTPCMap.h"

class TClonesArray;
class R3BGTPCHitPar;

class R3BGTPCCal2Hit : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCCal2Hit();

    /** Destructor **/
    ~R3BGTPCCal2Hit();

    /** Virtual method Exec **/
    virtual void Exec(Option_t* opt);

    /** Virtual method Reset **/
    virtual void Reset();

    /** Virtual method SetParContainers **/
    virtual void SetParContainers();

    /** Virtual method Init **/
    virtual InitStatus Init();

    /** Virtual method ReInit **/
    virtual InitStatus ReInit();

    /** Virtual method Finish **/
    virtual void Finish();

    /** Accessor to select online mode **/
    void SetOnline(Bool_t option) { fOnline = option; }

    typedef boost::multi_array<double, 3> multiarray;
    typedef multiarray::index index;
    multiarray PadCoordArr;

  private:
    void SetParameter();

    // TArrayF* fHitParams;
    // or maybe
    // Double_t fHitParam;

    R3BGTPCHitPar* fHit_Par; /**< Parameter container. >*/
    TClonesArray* fCalCA;
    TClonesArray* fHitCA;
    std::shared_ptr<R3BGTPCMap> fTPCMap;

    Bool_t fOnline; // Selector for online data storage

    /** Private method AddHitData**/
    //** Adds a Hit to the HitCollection
    R3BGTPCHitData* AddHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy);

    ClassDef(R3BGTPCCal2Hit, 1);
};
#endif
