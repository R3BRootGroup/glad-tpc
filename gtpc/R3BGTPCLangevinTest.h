// -----------------------------------------------------------------------------
// -----                  R3BGTPCLangevinTest header file                     -----
// -----                  Created 27/03/18  by H.Alvarez                   -----
// -----                Last modification 28/03/18                         -----
// -----------------------------------------------------------------------------

/**  R3BGTPCLangevinTest.h
 * Projects the electrons derived from the energy loss in the gas into
 * a virtual pad plane of variable size pads, including time information
 **/
#ifndef R3BGTPCLANGEVINTEST_H
#define R3BGTPCLANGEVINTEST_H

#include "TClonesArray.h"
#include "FairTask.h"
#include "R3BGTPCProjPoint.h"
#include "R3BGTPCPoint.h"
#include "TVirtualMC.h"


/**
 * GTPC drift calculation using Langevin equation task
 * @author Héctor Alvarez Pol
 *
 * For each event, get the R3BGTPCPoints and determine the projection on the pad plane
 *   Input:  Branch GTPCPoints = TClonesArray("R3BGTPCPoint")
 *   Output: Branch GTPCProjPoint = TClonesArray("R3BGTPCProjPoint")
 */

class R3BGTPCLangevinTest : public FairTask {
 public:

  /** Default constructor **/
  R3BGTPCLangevinTest();

  /** Destructor **/
  ~R3BGTPCLangevinTest();

  /** Virtual method Exec **/
  void Exec(Option_t*);

  /** Set parameters -- To be removed when parameter containers are ready **/
  void SetDriftParameters(Double_t ion, Double_t driftv,
        Double_t tDiff, Double_t lDiff,
        Double_t fanoFactor);

  void SetSizeOfVirtualPad(Double_t size);
  void SetNumberOfGeneratedElectrons(Double_t ele);

 protected:

  /** Virtual method Init **/
  InitStatus Init();

  /** Virtual method Finish **/
  void Finish();

  /** Virtual method SetParContainers **/
  void SetParContainers();

  TClonesArray* fGTPCPoints;
  TClonesArray* fGTPCProjPoint;

  //R3BGTPCGeoPar* fGTPCGeoPar; //!< Geometry parameter container (TODO)
  //R3BGTPCGasPar* fGTPCGasPar; //!< Gas parameter container (TODO)

 private:

  //Mapping of  virtualPadID to ProjPoint object pointer
  //std::map<Int_t, R3BGTPCProjPoint*> fProjPointMap;

  Double_t fEIonization;   //!< Effective ionization energy of gas [eV] NOTUSED
  Double_t fDriftVelocity; //!< Drift velocity in gas [cm/ns]
  Double_t fTransDiff;     //!< Transversal diffusion coefficient [cm^2/ns]
  Double_t fLongDiff;      //!< Longitudinal diffusion coefficient [cm^2/ns]
  Double_t fFanoFactor;    //!< Fano factor to calculate electron number fluctuations NOTUSED
  Double_t fHalfSizeTPC_X; //!< Half size X of the TPC drift volume [cm]
  Double_t fHalfSizeTPC_Y; //!< Half size Y of the TPC drift volume [cm]
  Double_t fHalfSizeTPC_Z; //!< Half size Z of the TPC drift volume [cm]
  Double_t fSizeOfVirtualPad;  //!< Number of virtual pad division per cm (default 1)

  Int_t fNumberOfGeneratedElectrons;  //!< Number of electrons to generate in each point of the test

  ClassDef(R3BGTPCLangevinTest, 1)
};

#endif // R3BGTPCLANGEVINTEST_H
