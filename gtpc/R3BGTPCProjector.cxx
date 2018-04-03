#include "R3BGTPCProjector.h"

#include "TClonesArray.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "R3BGTPCSetup.h"

R3BGTPCProjector::R3BGTPCProjector() : FairTask("R3BGTPCProjector") {
  fEIonization = 15.e-9;  // [GeV]
  fDriftVelocity = 0.0005; // [cm/ns]
  fTransDiff = 0.00000001; // [cm^2/ns]
  fLongDiff  = 0.0000001;  // [cm^2/ns]
  fFanoFactor = 2; 
  fHalfSizeTPC_X = 0.;  //to be filled during Init()
  fHalfSizeTPC_Y = 0.;  //to be filled during Init()
  fHalfSizeTPC_Z = 0.;  //to be filled during Init()
  fSizeOfVirtualPad = 1.; //1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
}

R3BGTPCProjector::~R3BGTPCProjector(){
  fGTPCPoints->Clear();
  fGTPCProjPoint->Clear();
}

void R3BGTPCProjector::SetParContainers() {
  FairRunAna* run = FairRunAna::Instance();
  if (!run){
    LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No analysis run" << FairLogger::endl;
    return;
  }
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  if (!rtdb) {
    LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No runtime database" << FairLogger::endl;
    return;
  }
  //PREPARE PROPER PARAMETER CONTAINERS
  /*fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("R3BGTPCGeoPar");
    if (!fGTPCGeoPar) {
    LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No R3BGTPCGeoPar" << FairLogger::endl;
    return;
    }
  */
  /*fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("R3BGTPCGasPar");
    if (!fGTPCGasPar) {
    LOG(FATAL) << "R3BGTPCProjector::SetParContainers: No R3BGTPCGasPar" << FairLogger::endl;
    return;
    }
  */
}

InitStatus R3BGTPCProjector::Init() {
  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman) {
    LOG(FATAL) << "R3BGTPCProjector::Init: No FairRootManager" << FairLogger::endl;
    return kFATAL;
  }
  // Input: TClonesArray of R3BGTPCPoints
  if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr) {
    LOG(FATAL) << "R3BGTPCProjector::Init No GTPCPoint!" << FairLogger::endl;
    return kFATAL;
  }
  fGTPCPoints = (TClonesArray*)ioman->GetObject("GTPCPoint");
  
  // Output: TClonesArray of R3BGTPCProjPoint
  fGTPCProjPoint = new TClonesArray("R3BGTPCProjPoint");
  ioman->Register("GTPCProjPoint", GetName(), fGTPCProjPoint, kTRUE);
  
  /*   //PREPARE PROPER PARAMETER CONTAINERS FOR DRIFT PARAMETERS
       fEIonize  = fPar->GetEIonize()/1000000000; // [GeV] 
       fVelDrift = fPar->GetDriftVelocity(); // [cm/us]
       fCoefT    = fPar->GetCoefDiffusionTrans()*sqrt(10.); // [cm^(-1/2)] to [mm^(-1/2)]
       fCoefL    = fPar->GetCoefDiffusionLong()*sqrt(10.);  // [cm^(-1/2)] to [mm^(-1/2)]
  */
  
  R3BGTPCSetup* setup = new R3BGTPCSetup(1);
  fHalfSizeTPC_X = setup->GetTPCLx()/2.;
  fHalfSizeTPC_Y = setup->GetTPCLy()/2.;
  fHalfSizeTPC_Z = setup->GetTPCLz()/2.;
  delete setup;
  
  return kSUCCESS;
}

void R3BGTPCProjector::SetDriftParameters(Double_t ion, Double_t driftv, 
					  Double_t tDiff, Double_t lDiff, 
					  Double_t fanoFactor) {
  fEIonization = ion;       // [eV]    15/1000000000; [GeV]
  fDriftVelocity = driftv;  // [mm/ns]
  fTransDiff = tDiff;       // [mm^(-1/2)] ~0.010
  fLongDiff = lDiff;        // [mm^(-1/2)] ~0.025
  fFanoFactor = fanoFactor; //
}

void R3BGTPCProjector::Exec(Option_t*) {
      
  fGTPCProjPoint->Clear("C");
  
  Int_t nPoints = fGTPCPoints->GetEntries();
  LOG(INFO) << "R3BGTPCProjector: processing " << nPoints << " points" << FairLogger::endl;
  if(nPoints<2){
    LOG(INFO) << "Not enough hits for digitization! (<2)" << FairLogger::endl;
    return;
  }  

  R3BGTPCPoint* aPoint; 
  //R3BGTPCProjPoint* aProjPoint; 
  Int_t presentTrackID = -10; //control of the point trackID 
  Double_t xPre, yPre, zPre; Double_t xPost, yPost, zPost; Double_t projX, projZ, projTime;
  Double_t energyDep = 0.; Double_t timeBeforeDrift = 0.;
  Bool_t readyToProject = kFALSE; Bool_t virtualPadFound = kFALSE;
  Int_t electrons = 0; Int_t flucElectrons = 0; Int_t generatedElectrons = 0;
  Double_t stepX, stepY, stepZ;
  Double_t driftDistance, driftTime;
  Double_t sigmaLongAtPadPlane;  Double_t sigmaTransvAtPadPlane; 
  Int_t evtID;
  
  for(Int_t i=0; i<nPoints; i++) {
    aPoint = (R3BGTPCPoint*) fGTPCPoints->At(i);
    evtID=aPoint->GetEventID();
    if(aPoint->GetTrackStatus() == 11000 || 
       aPoint->GetTrackStatus() == 10010010  || 
       aPoint->GetTrackStatus() == 10010000) { 
      //entering the gas volume or new track inside the gas (is 10010010 or 10010000??)
      presentTrackID = aPoint->GetTrackID();
      xPre = aPoint->GetX(); yPre = aPoint->GetY(); zPre = aPoint->GetZ();
      readyToProject = kTRUE;
      continue; //no energy deposited in this point, just taking in entrance coordinates
    }
    else { //any other case
      if(presentTrackID != aPoint->GetTrackID()) {
	LOG(FATAL) << "R3BGTPCProjector::Exec: Problem 2 in point logic" << FairLogger::endl;
	break;
      }
      if(readyToProject!=kTRUE) {
	LOG(FATAL) << "R3BGTPCProjector::Exec: Problem 3 in point logic" << FairLogger::endl;
	break;
      }
      if (aPoint->GetTrackStatus() == 10100 || 
	  aPoint->GetTrackStatus() == 1000000 ) { //exiting the gas volume or dissappearing
	readyToProject = kFALSE;
      }	
      
      xPost = aPoint->GetX(); yPost = aPoint->GetY(); zPost = aPoint->GetZ();   
      energyDep = aPoint->GetEnergyLoss();
      timeBeforeDrift = aPoint->GetTime(); 
    }
    
    electrons = energyDep/fEIonization;
    //electron number fluctuates as the square root of the 
    //Fano factor times the number of electrons
    flucElectrons = pow(fFanoFactor*electrons, 0.5); 
    generatedElectrons = gRandom->Gaus(electrons, flucElectrons); //generated electrons
    
    //step in each direction for an homogeneous electron creation position along the track
    stepX = (xPost-xPre) / generatedElectrons;
    stepY = (yPost-yPre) / generatedElectrons;
    stepZ = (zPost-zPre) / generatedElectrons;
    
    //taken a mean driftDistance for the calculation of the sigmaLong and sigmaTrans
    //improve (make the calculation individual for electron) if needed, but probably slower
    Double_t yApprox = (yPost+yPre)/2;
    driftDistance = yApprox + fHalfSizeTPC_Y;
    sigmaLongAtPadPlane = sqrt(driftDistance*2*fLongDiff/fDriftVelocity);
    sigmaTransvAtPadPlane = sqrt(driftDistance*2*fTransDiff/fDriftVelocity);

    for(Int_t ele=1;ele<=generatedElectrons;ele++){
      driftTime = ((yPre+stepY*ele)+fHalfSizeTPC_Y)/fDriftVelocity;
      projX =  gRandom->Gaus(xPre+stepX*ele,sigmaTransvAtPadPlane);
      projZ =  gRandom->Gaus(zPre+stepZ*ele,sigmaTransvAtPadPlane);
      projTime = gRandom->Gaus(driftTime+timeBeforeDrift,sigmaLongAtPadPlane/fDriftVelocity);

      //obtain padID for projX, projZ (simple algorithm) 
      //the algorithm assigns a pad number which depends on the XZ size of the chamber, 
      //according to the fSizeOfVirtualPad parameter: if it is 1, the pad size is cm^2
      //and padID goes from 0 to 2*fHalfSizeTPC_X in the first row (Z~200), 
      //from 2*fHalfSizeTPC_X to 4*fHalfSizeTPC_X in the second row (Z~201), ...
      //if fSizeOfVirtualPad=0.1, then padID goes from 0 to 20*fHalfSizeTPC_X for (Z~200.0),
      //from 20*fHalfSizeTPC_X to 40*fHalfSizeTPC_X  (Z~200.0), ...
      //Avoid first moving out of the virtual pad plane limits
      if(projZ<200) projZ=200.0; 
      if(projZ>200+2*fHalfSizeTPC_Z) projZ=200.0+2*fHalfSizeTPC_Z;
      if(projX<-fHalfSizeTPC_X) projX=-fHalfSizeTPC_X; 
      if(projX>2*fHalfSizeTPC_X) projX=2*fHalfSizeTPC_X;
      Int_t padID = (2*fHalfSizeTPC_X*fSizeOfVirtualPad) * 
	(Int_t)((projZ-200.)*fSizeOfVirtualPad) + 
	(Int_t)((projX+fHalfSizeTPC_X)*fSizeOfVirtualPad);
      
      Int_t nProjPoints = fGTPCProjPoint->GetEntriesFast();
      for(Int_t pp=0; pp<nProjPoints; pp++) {
	if(((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->GetVirtualPadID() ==  padID){
	  //already existing R3BGTPCProjPoint... add time and electron
	  ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->AddCharge();
	  ((R3BGTPCProjPoint*)fGTPCProjPoint->At(pp))->SetTimeDistr(projTime/1000,1);//micros
	  virtualPadFound=kTRUE;
	  break;
	}
      }
      if(!virtualPadFound){
	new ((*fGTPCProjPoint)[nProjPoints]) R3BGTPCProjPoint(padID, projTime, 1, evtID);
      }
      virtualPadFound = kFALSE;
    }
    xPre = xPost; yPre = yPost; zPre = zPost;
  }
  LOG(INFO) << "R3BGTPCProjector: produced " << fGTPCProjPoint->GetEntries() << " projPoints" << FairLogger::endl;
}


void R3BGTPCProjector::Finish() {
  
}

ClassImp(R3BGTPCProjector)
