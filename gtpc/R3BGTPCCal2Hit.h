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
#include "R3BGTPCElecPar.h"
#include "R3BGTPCGasPar.h"
#include "R3BGTPCGeoPar.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCMap.h"

class TClonesArray;

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
    void SetRecoFlag(Bool_t BooleanFlag) { fLangevinBack = BooleanFlag; }

    typedef boost::multi_array<double, 3> multiarray;
    typedef multiarray::index index;
    multiarray PadCoordArr;

  private:
    void SetParameter();

    Double_t fEIonization;      //!< Effective ionization energy of gas [GeV]
    Double_t fDriftVelocity;    //!< Drift velocity in gas [cm/ns]
    Double_t fTransDiff;        //!< Transversal diffusion coefficient [cm^2/ns]
    Double_t fLongDiff;         //!< Longitudinal diffusion coefficient [cm^2/ns]
    Double_t fFanoFactor;       //!< Fano factor to calculate electron number fluctuations
    Double_t fHalfSizeTPC_X;    //!< Half size X of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Y;    //!< Half size Y of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Z;    //!< Half size Z of the TPC drift volume [cm]
    Double_t fSizeOfVirtualPad; //!< Number of virtual pad division per cm (default 1)
    Double_t fDriftEField;      //!< Drift electric field [V/cm]
    Double_t fDriftTimeStep;    //!< Time Step between drift parameters calculation [ns]
    Double_t fTimeBinSize;      //!< Time size of each bin in the time vector [ns]
    Double_t fOffsetX;          //!< X offset for prototype [cm] --> Warn: Make sure
                                //!< run_reconstruction.C uses the correct .par
    Double_t fOffsetZ;          //!< Z offset for prototype [cm] --> Warn: Make sure
                                //!< run_reconstruction.C uses the correct .par
    Double_t fTargetOffsetX;
    Double_t fTargetOffsetY;
    Double_t fTargetOffsetZ;

    Int_t fDetectorType; //!< Detector type: 1 for prototype, 2 for FullBeamIn, 3
                         //!< for FullBeamOut

    R3BGTPCGeoPar* fGTPCGeoPar;   //!< Geometry parameter container
    R3BGTPCGasPar* fGTPCGasPar;   //!< Gas parameter container
    R3BGTPCElecPar* fGTPCElecPar; //!< Electronic parameter container

    TClonesArray* fCalCA;
    TClonesArray* fHitCA;
    std::shared_ptr<R3BGTPCMap> fTPCMap;

    Bool_t fOnline; // Selector for online data storage

    Bool_t fLangevinBack; // Flag for my reconstruction (David)
    // True: Reconstruction with Langevin equations
    // False: Reconstruction already done

    /** Private method AddHitData**/
    //** Adds a Hit to the HitCollection
    R3BGTPCHitData* AddHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy);

    ClassDef(R3BGTPCCal2Hit, 1);
};
