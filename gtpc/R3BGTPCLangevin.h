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

/**  R3BGTPCLangevin.h
 * Projects the electrons derived from the energy loss in the gas into
 * a virtual pad plane of variable size pads, including time information
 **/

#pragma once

#include "FairTask.h"
#include "R3BGTPCCalData.h"
#include "R3BGTPCElecPar.h"
#include "R3BGTPCGasPar.h"
#include "R3BGTPCGeoPar.h"
#include "R3BGTPCMap.h"
#include "R3BGTPCPoint.h"
#include "R3BGTPCProjPoint.h"
#include "TClonesArray.h"
#include "TVirtualMC.h"

/**
 * GTPC drift calculation using Langevin equation task
 * @author Héctor Alvarez Pol
 *
 * For each event, get the R3BGTPCPoints and determine the projection on the pad
 * plane Input:  Branch GTPCPoints = TClonesArray("R3BGTPCPoint") Output: Branch
 * GTPCCalData = TClonesArray("R3BGTPCCalData") Output: Branch GTPCProjPoint =
 * TClonesArray("R3BGTPCProjPoint")
 */

class R3BGTPCLangevin : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCLangevin();

    /** Destructor **/
    ~R3BGTPCLangevin();

    /** Virtual method Init **/
    virtual InitStatus Init();

    /** Virtual method Exec **/
    void Exec(Option_t*);

    /** Virtual method ReInit **/
    virtual InitStatus ReInit();

    /** Virtual method Finish **/
    void Finish();

    /** Virtual method SetParContainers **/
    void SetParContainers();

    void SetParameter();

    void SetProjPointsAsOutput() { outputMode = 1; }
    void SetCalDataAsOutput() { outputMode = 0; }

  private:
    // Mapping of  virtualPadID to ProjPoint object pointer
    // std::map<Int_t, R3BGTPCProjPoint*> fProjPointMap;

    Double_t fEIonization;      //!< Effective ionization energy of gas [eV]
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
                                //!< run_lang.C uses the correct .par
    Double_t fOffsetZ;          //!< Z offset for prototype [cm] --> Warn: Make sure
                                //!< run_lang.C uses the correct .par

    Int_t fDetectorType; //!< Detector type: 1 for prototype, 2 for FullBeamIn, 3
                         //!< for FullBeamOut

    R3BGTPCGeoPar* fGTPCGeoPar;   //!< Geometry parameter container
    R3BGTPCGasPar* fGTPCGasPar;   //!< Gas parameter container
    R3BGTPCElecPar* fGTPCElecPar; //!< Electronic parameter container

    Int_t outputMode; //!< Selects Cal(0) or ProjPoint(1) as output level. Default 0
    TClonesArray* fGTPCPointsCA;
    TClonesArray* fGTPCCalDataCA;
    TClonesArray* fGTPCProjPointCA;
    // MCTrack- vertex information
    TClonesArray* fMCTrackCA;

    // R3BGTPCCalData* AddCalData();

    std::shared_ptr<R3BGTPCMap> fTPCMap; //!< Map container
    TH2Poly* fPadPlane;                  //!< Pad Plane object

    ClassDef(R3BGTPCLangevin, 2)
};
