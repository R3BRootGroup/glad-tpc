/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

/** R3BTPCProjector.h
 *
 * Projects ionisation electrons from GTPCPoint objects onto a configurable
 * virtual pad plane.
 *
 * Geometry policy:
 *   - Active_region size, translation and rotation are taken from TGeo.
 *   - GTPCPoint positions are transformed from master (global) coordinates
 *     into the local Active_region coordinate system.
 *   - Drift is along local +/-Y.
 *   - The readout plane is the local X-Z face.
 *
 * Pad-map policy:
 *   - R3BGTPCMap owns pad geometry, pitch/gap handling, pad numbering,
 *     TH2Poly generation and position -> pad lookup.
 *   - The projector only stores the user-requested segmentation until Init(),
 *     when the Active_region X/Z dimensions are known.
 *
 * Input : GTPCPoint (R3BGTPCPoint), MCTrack (R3BMCTrack)
 * Output: GTPCProjPoint (R3BGTPCProjPoint) or GTPCCalData (R3BGTPCCalData)
 */

#pragma once

#include "FairTask.h"

#include "R3BGTPCCalData.h"
#include "R3BGTPCElecPar.h"
#include "R3BGTPCGasPar.h"
#include "R3BGTPCMap.h"
#include "R3BGTPCPoint.h"
#include "R3BGTPCProjPoint.h"

#include "TClonesArray.h"
#include "TGeoMatrix.h"
#include "TH2Poly.h"

#include <memory>

class R3BTPCProjector : public FairTask
{
  public:
    /** Default constructor */
    R3BTPCProjector();

    /** Destructor */
    ~R3BTPCProjector();

    /** Event-by-event projection */
    void Exec(Option_t*);

    /** Optional compatibility/testing override.
     *  Normal production use reads these quantities from GTPCGasPar. */
    void SetDriftParameters(Double_t, Double_t, Double_t, Double_t, Double_t);

    /** Output level */
    void SetProjPointsAsOutput() { outputMode = 1; }
    void SetCalDataAsOutput() { outputMode = 0; }

    /** Readout plane: +1 = local +Y, -1 = local -Y */
    void SetReadoutSide(Int_t side);
    Int_t GetReadoutSide() const { return fReadoutSide; }

    /** User-defined readout segmentation (all lengths in cm).
     *  Active X/Z dimensions are obtained automatically from Active_region. */
    void SetPadGeometry(Int_t rowsX,
                        Int_t columnsZ,
                        Double_t padSizeXcm,
                        Double_t padSizeZcm,
                        Double_t gapXcm,
                        Double_t gapZcm);

    TH2Poly* GetPadPlane() { return fTPCMap ? fTPCMap->GetPadPlane() : nullptr; }

    std::shared_ptr<R3BGTPCMap> GetTPCMap() const { return fTPCMap; }

    const TGeoHMatrix& GetActiveLocalToMaster() const { return fActiveLocalToMaster; }

    Double_t GetActiveMinX() const { return fActiveMinX; }

    Double_t GetActiveMinZ() const { return fActiveMinZ; }

    Double_t GetReadoutPlaneLocalY() const { return (fReadoutSide > 0) ? fActiveMaxY : fActiveMinY; }

  protected:
    virtual InitStatus Init();
    virtual InitStatus ReInit();
    void Finish();
    void SetParContainers();
    void SetParameter();

    TClonesArray* fGTPCPoints;    //!< Input  : GTPCPoint
    TClonesArray* fGTPCProjPoint; //!< Output : GTPCProjPoint
    TClonesArray* fGTPCMapOutput; //!< Output : configured R3BGTPCMap snapshot
    TClonesArray* fGTPCCalDataCA; //!< Output : GTPCCalData
    TClonesArray* MCTrackCA;      //!< Input  : MCTrack

  private:
    /** Locate Active_region and cache its local bounds and placement matrix */
    Bool_t ConfigureFromGeometry();

    // ---- Gas / transport properties
    Double_t fEIonization;   //!< Mean ion-pair energy [GeV]
    Double_t fDriftVelocity; //!< Drift velocity [cm/ns]
    Double_t fTransDiff;     //!< Transverse diffusion coefficient [cm^2/ns]
    Double_t fLongDiff;      //!< Longitudinal diffusion coefficient [cm^2/ns]
    Double_t fFanoFactor;    //!< Fano factor

    // ---- Electronics quantities used at projector level
    Double_t fTimeBinSize;   //!< Time bin size [ns]
    Double_t fDriftEField;   //!< Drift field [V/cm]
    Double_t fDriftTimeStep; //!< Drift time step [ns]

    Int_t outputMode;   //!< 0 = CalData, 1 = ProjPoint
    Int_t fReadoutSide; //!< +1 = local +Y, -1 = local -Y

    // ---- Active_region bounds in LOCAL coordinates [cm]
    Double_t fActiveMinX;
    Double_t fActiveMaxX;
    Double_t fActiveMinY;
    Double_t fActiveMaxY;
    Double_t fActiveMinZ;
    Double_t fActiveMaxZ;

    Double_t fActiveSizeX;
    Double_t fActiveSizeY;
    Double_t fActiveSizeZ;

    // ---- Placement of Active_region
    TGeoHMatrix fActiveLocalToMaster; //!< Local -> master matrix
    Bool_t fGeometryConfigured;       //!< Set by ConfigureFromGeometry()

    // ---- User-selected pad segmentation, passed to R3BGTPCMap in Init()
    Int_t fPadRowsX;
    Int_t fPadColumnsZ;
    Double_t fPadSizeXcm;
    Double_t fPadSizeZcm;
    Double_t fPadGapXcm;
    Double_t fPadGapZcm;
    Bool_t fPadGeometryConfigured;

    // ---- True only when SetDriftParameters() was explicitly used
    Bool_t fUseManualDriftParameters;

    // ---- Parameter containers and pad map
    R3BGTPCGasPar* fGTPCGasPar;          //!< Gas parameter container
    R3BGTPCElecPar* fGTPCElecPar;        //!< Electronics parameter container
    std::shared_ptr<R3BGTPCMap> fTPCMap; //!< Pad map

    ClassDef(R3BTPCProjector, 3)
};
