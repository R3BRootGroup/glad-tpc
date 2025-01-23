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

/**  R3BGTPCProjector.h
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

/**
 * GTPC point projector task
 * @author Héctor Alvarez Pol
 *
 * For each event, get the R3BGTPCPoints and determine the projection on the pad
 * plane Input:  Branch GTPCPoints = TClonesArray("R3BGTPCPoint") Output: Branch
 * GTPCProjPoint = TClonesArray("R3BGTPCProjPoint")
 *
 * Updated (@author Yassid Ayyad)
 *  Added R3BGTPCMap as map manager
 *  Output: Branch GTPCCalData = TClonesArray("R3BGTPCCalData")
 */

class R3BGTPCProjector : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCProjector();

    /** Destructor **/
    ~R3BGTPCProjector();

    /** Virtual method Exec **/
    void Exec(Option_t*);

    /** Set parameters -- To be removed when parameter containers are ready **/
    void SetDriftParameters(Double_t ion, Double_t driftv, Double_t tDiff, Double_t lDiff, Double_t fanoFactor);

    void SetProjPointsAsOutput() { outputMode = 1; }
    void SetCalDataAsOutput() { outputMode = 0; }

  protected:
    /** Virtual method Init **/
    virtual InitStatus Init();

    /** Virtual method ReInit **/
    virtual InitStatus ReInit();

    /** Virtual method Finish **/
    void Finish();

    /** Virtual method SetParContainers **/
    void SetParContainers();

    void SetParameter();

    TClonesArray* fGTPCPoints;
    TClonesArray* fGTPCProjPoint;
    TClonesArray* fGTPCCalDataCA;
    // MCTrack- vertex information
    TClonesArray* MCTrackCA;

  private:
    // Mapping of  virtualPadID to ProjPoint object pointer
    // std::map<Int_t, R3BGTPCProjPoint*> fProjPointMap;

    Double_t fEIonization;   //!< Effective ionization energy of gas [eV]
    Double_t fDriftVelocity; //!< Drift velocity in gas [cm/ns]
    Double_t fTransDiff;     //!< Transversal diffusion coefficient [cm^2/ns]
    Double_t fLongDiff;      //!< Longitudinal diffusion coefficient [cm^2/ns]
    Double_t fFanoFactor;    //!< Fano factor to calculate electron number fluctuations
    Double_t fHalfSizeTPC_X; //!< Half size X of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Y; //!< Half size Y of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Z; //!< Half size Z of the TPC drift volume [cm]
    Double_t fTimeBinSize;   //!< Time size of each bin in the time vector [ns]
    Double_t fOffsetX;       //!< Offset X between the active target and the padplane
    Double_t fOffsetZ;       //!< Offset Z between the active target and the padplane
    Double_t fDriftEField;   //!< Drift electric field [V/cm]
    Double_t fDriftTimeStep; //!< Time Step between drift parameters calculation [ns]
    Int_t fDetectorType;     //!< Detector type: 1 for prototype, 2 for FullBeamIn, 3
                             //!< for FullBeamOut
    Int_t outputMode;        //!< Selects Cal(0) or ProjPoint(1) as output level. Default 0

    R3BGTPCGeoPar* fGTPCGeoPar;   //!< Geometry parameter container
    R3BGTPCGasPar* fGTPCGasPar;   //!< Gas parameter container
    R3BGTPCElecPar* fGTPCElecPar; //!< Electronics parameter container

    std::shared_ptr<R3BGTPCMap> fTPCMap; //!< Map container
    TH2Poly* fPadPlane;                  //!< Pad Plane object

    ClassDef(R3BGTPCProjector, 1)
};
