// -----------------------------------------------------------------------------
// -----                  R3BGTPCProjector header file                     -----
// -----                  Created 27/03/18  by H.Alvarez                   -----
// -----           	Last modification 16/02/2021 S. Velardita              -----
/// -----------------------------------------------------------------------------
//
/**  R3BGTPCProjector.h
 * Projects the electrons derived from the energy loss in the gas into
 * a virtual pad plane of variable size pads, including time information
 **/
#ifndef R3BGTPCPROJECTOR_H
#define R3BGTPCPROJECTOR_H

#include "FairTask.h"
#include "R3BGTPCPoint.h"
#include "R3BGTPCProjPoint.h"
#include "TClonesArray.h"

/**
 * GTPC point projector task
 * @author Héctor Alvarez Pol
 *
 * For each event, get the R3BGTPCPoints and determine the projection on the pad plane
 *   Input:  Branch GTPCPoints = TClonesArray("R3BGTPCPoint")
 *   Output: Branch GTPCProjPoint = TClonesArray("R3BGTPCProjPoint")
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

    void SetSizeOfVirtualPad(Double_t size);
    

  protected:
    /** Virtual method Init **/
    InitStatus Init();

    /** Virtual method Finish **/
    void Finish();

    /** Virtual method SetParContainers **/
    void SetParContainers();

    TClonesArray* fGTPCPoints;
    TClonesArray* fGTPCProjPoint;

    // R3BGTPCGeoPar* fGTPCGeoPar; //!< Geometry parameter container (TODO)
    // R3BGTPCGasPar* fGTPCGasPar; //!< Gas parameter container (TODO)

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

    ClassDef(R3BGTPCProjector, 1)
};

#endif // R3BGTPCPROJECTOR_H
