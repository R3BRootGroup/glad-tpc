/*******************************************************************************
 * Copyright (C) 2024 GSI Helmholtzzentrum für Schwerionenforschung GmbH       *
 * Copyright (C) 2024-2025 Members of the R3B Collaboration                    *
 *                                                                             *
 *             This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,            *
 *                   copied verbatim in the file "LICENSE".                    *
 *                                                                             *
 *  In applying this license GSI does not waive the privileges and immunities  *
 *  granted to it by virtue of its status as an Intergovernmental Organization *
 *  or submit itself to any jurisdiction.                                      *
 ********************************************************************************/

/**  R3BGTPCLaserGen.h
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
 * GTPCProjPoint = TClonesArray("R3BGTPCProjPoint")
 */

class R3BGTPCLaserGen : public FairTask
{
  public:
    /** Default constructor **/
    R3BGTPCLaserGen();

    /** Destructor **/
    ~R3BGTPCLaserGen();

    /** Virtual method Exec **/
    void Exec(Option_t*);

    /** Set parameters -- To be removed when parameter containers are ready **/
    void SetDriftParameters(Double_t ion, Double_t driftv, Double_t tDiff, Double_t lDiff, Double_t fanoFactor);
    void SetLaserParameters(Double_t alpha,
                            Double_t beta,
                            Double_t x_in,
                            Double_t y_in,
                            Double_t z_in,
                            Double_t maxLength);
    void SetSizeOfVirtualPad(Double_t size);
    void SetNumberOfGeneratedElectrons(Double_t ele);
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

    TClonesArray* fGTPCPointsCA;
    TClonesArray* fGTPCProjPointCA;
    TClonesArray* fGTPCCalDataCA;
    // MCTrack- vertex information
    TClonesArray* MCTrackCA;

  private:
    // Mapping of  virtualPadID to ProjPoint object pointer

    void SetParameter();

    std::map<Int_t, R3BGTPCProjPoint*> fProjPointMap;

    Double_t fEIonization;      //!< Effective ionization energy of gas [eV] NOTUSED
    Double_t fDriftVelocity;    //!< Drift velocity in gas [cm/ns]
    Double_t fTransDiff;        //!< Transversal diffusion coefficient [cm^2/ns]
    Double_t fLongDiff;         //!< Longitudinal diffusion coefficient [cm^2/ns]
    Double_t fFanoFactor;       //!< Fano factor to calculate electron number
                                //!< fluctuations NOTUSED
    Double_t fHalfSizeTPC_X;    //!< Half size X of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Y;    //!< Half size Y of the TPC drift volume [cm]
    Double_t fHalfSizeTPC_Z;    //!< Half size Z of the TPC drift volume [cm]
    Double_t fSizeOfVirtualPad; //!< Number of virtual pad division per cm (default 1)
    Double_t fDriftEField;
    Double_t fDriftTimeStep;
    Double_t fTimeBinSize;
    Double_t fAlpha;
    Double_t fBeta;
    Double_t fXIn;
    Double_t fYIn;
    Double_t fZIn;
    Double_t fMaxLength;
    Double_t fOffsetX;
    Double_t fOffsetZ;
    Double_t fTargetOffsetX;
    Double_t fTargetOffsetY;
    Double_t fTargetOffsetZ;

    Int_t outputMode; //!< Selects Cal(0) or ProjPoint(1) as output level. Default 0

    Int_t fNumberOfGeneratedElectrons; //!< Number of electrons to generate in
                                       //!< each point of the test

    R3BGTPCGeoPar* fGTPCGeoPar;   //!< Geometry parameter container
    R3BGTPCGasPar* fGTPCGasPar;   //!< Gas parameter container
    R3BGTPCElecPar* fGTPCElecPar; //!< Electronic parameter container

    std::shared_ptr<R3BGTPCMap> fTPCMap;
    TH2Poly* fPadPlane;

    ClassDef(R3BGTPCLaserGen, 1)
};
