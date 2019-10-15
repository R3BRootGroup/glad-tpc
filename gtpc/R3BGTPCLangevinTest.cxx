#include "R3BGTPCLangevinTest.h"

#include "TClonesArray.h"
#include "TMath.h"
//#include "TVirtualMC.h"
//#include "TVirtualMagField.h"

#include "R3BGladFieldMap.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "R3BGTPCSetup.h"

R3BGTPCLangevinTest::R3BGTPCLangevinTest() : FairTask("R3BGTPCLangevinTest") {
  //ALL UNITS IN cm, ns, V/cm, Tesla and GeV
  fEIonization = 15.e-9;   // [GeV] NOTUSED
  fDriftVelocity = 0.0048; // [cm/ns] just initial value
  fTransDiff = 0.00000216; // [cm^2/ns] just initial value
  fLongDiff  = 0.00000216;  // [cm^2/ns] just initial value
  fFanoFactor = 2; //NOTUSED
  fHalfSizeTPC_X = 40.;  //[cm] to be used with the output of create_tpc_geo_test.C
  fHalfSizeTPC_Y = 20.;  //
  fHalfSizeTPC_Z = 100.;  //
  fSizeOfVirtualPad = 100.; //1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
}

R3BGTPCLangevinTest::~R3BGTPCLangevinTest(){
  fGTPCPoints->Clear();
  fGTPCProjPoint->Clear();
}

void R3BGTPCLangevinTest::SetParContainers() {
  FairRunAna* run = FairRunAna::Instance();
  if (!run){
    LOG(FATAL) << "R3BGTPCLangevinTest::SetParContainers: No analysis run";
    return;
  }
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  if (!rtdb) {
    LOG(FATAL) << "R3BGTPCLangevinTest::SetParContainers: No runtime database";
    return;
  }
  //PREPARE PROPER PARAMETER CONTAINERS
  /*fGTPCGeoPar = (R3BGTPCGeoPar*)rtdb->getContainer("R3BGTPCGeoPar");
    if (!fGTPCGeoPar) {
    LOG(FATAL) << "R3BGTPCLangevinTest::SetParContainers: No R3BGTPCGeoPar";
    return;
    }
  */
  /*fGTPCGasPar = (R3BGTPCGasPar*)rtdb->getContainer("R3BGTPCGasPar");
    if (!fGTPCGasPar) {
    LOG(FATAL) << "R3BGTPCLangevinTest::SetParContainers: No R3BGTPCGasPar";
    return;
    }
  */
}

InitStatus R3BGTPCLangevinTest::Init() {
  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman) {
    LOG(FATAL) << "R3BGTPCLangevinTest::Init: No FairRootManager";
    return kFATAL;
  }
  // Input: TClonesArray of R3BGTPCPoints
  if ((TClonesArray*)ioman->GetObject("GTPCPoint") == nullptr) {
    LOG(FATAL) << "R3BGTPCLangevinTest::Init No GTPCPoint!";
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

  /*
  R3BGTPCSetup* setup = new R3BGTPCSetup(1);
  fHalfSizeTPC_X = setup->GetTPCLx()/2.;
  fHalfSizeTPC_Y = setup->GetTPCLy()/2.;
  fHalfSizeTPC_Z = setup->GetTPCLz()/2.;
  delete setup;
  */

  return kSUCCESS;
}

void R3BGTPCLangevinTest::SetDriftParameters(Double_t ion, Double_t driftv,
					  Double_t tDiff, Double_t lDiff,
					  Double_t fanoFactor) {
  fEIonization = ion;       // [GeV] NOTUSED
  fDriftVelocity = driftv;  // [cm/ns]
  fTransDiff = tDiff;       // [cm^(-1/2)]
  fLongDiff = lDiff;        // [cm^(-1/2)]
  fFanoFactor = fanoFactor; // NOTUSED
}

void R3BGTPCLangevinTest::Exec(Option_t*) {

  fGTPCProjPoint->Clear("C");

  LOG(INFO) << "R3BGTPCLangevinTest: test";
  Int_t nPoints = fGTPCPoints->GetEntries();

  //if(nPoints<2){
  //  LOG(INFO) << "Not enough hits for digitization! (<2)";
  //  return;
  //}

  R3BGladFieldMap* gladField = (R3BGladFieldMap*)FairRunAna::Instance()->GetField();

  R3BGTPCPoint* aPoint;
  //R3BGTPCProjPoint* aProjPoint;
  Double_t projX, projZ, projTime;
  Double_t timeBeforeDrift = 0.;
  Bool_t virtualPadFound = kFALSE;
  Int_t generatedElectrons = 0;
  Double_t driftDistance;
  Double_t sigmaLongAtPadPlane;  Double_t sigmaTransvAtPadPlane;
  Int_t evtID;

  for(Int_t i=0; i<nPoints; i++) {
    aPoint = (R3BGTPCPoint*) fGTPCPoints->At(i);
    evtID=aPoint->GetEventID();
  }

  //Using here m, V, T, s just to simplify in SI system...
  Double_t E_x = 0; Double_t E_z = 0; Double_t E_y = 10000; //(100 V/cm) TODO check value, move to parameter container
  Double_t B_x = 0; Double_t B_y = 0; Double_t B_z = 0; Double_t moduleB = 0;
  Double_t ele_x_init = 0; Double_t ele_y_init = 0; Double_t ele_z_init = 0;
  Double_t ele_x = 0; Double_t ele_y = 0; Double_t ele_z = 0;
  Double_t vDrift_x = 0; Double_t vDrift_y = 0; Double_t vDrift_z = 0;
  Double_t accDriftTime = 0;
  Double_t cteMult = 0; Double_t cteMod = 0; Double_t productEB = 0;
  Double_t sigmaLongStep;  Double_t sigmaTransvStep;  Double_t driftTimeStep;
  Double_t mu = 1.E+5 * fDriftVelocity/E_y;  // [m2 s-1 V-1] TODO check value, move to parameter container

  //number of electrons per grid point
  generatedElectrons = 0; //1.E+03;

  //from create_tpc_geo_test.C (geo in file R3BRoot/glad-tpc/geometry/gladTPC_test.geo.root)
  Double_t TargetOffsetX=40;
  Double_t TargetOffsetY=0;
  Double_t TargetOffsetZ=260; // USING INSTEAD THE FIELD MAP DESPLACEMENT! MISMATCH
  Double_t TargetOffsetZ_FM=263.4; // FIELD MAP DESPLACEMENT

  Double_t TargetAngle = 14.*3.14159/180;

  B_x = 0.1 * gladField->GetBx(0, 0, 163.4);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(0, 0, 163.4);
  B_z = 0.1 * gladField->GetBz(0, 0, 163.4);
  cout << "Field for (0,0,163.4) in R3B coordinates or (0,0,0) in field map coordinates: " << B_x << " "  << B_y << " "  << B_z << " "  << endl;
  B_x = 0.1 * gladField->GetBx(-7.2837383, -10, 151.27782);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(-7.2837383, -10, 151.27782);
  B_z = 0.1 * gladField->GetBz(-7.2837383, -10, 151.27782);
  cout << "Field for (-7.2837383, -10, 151.27782) in R3B coordinates or (-10, -10, -10) in field map coordinates: " << B_x << " "  << B_y << " "  << B_z << " "  << endl;
  B_x = 0.1 * gladField->GetBx(-7.2837383, 10, 151.27782);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(-7.2837383, 10, 151.27782);
  B_z = 0.1 * gladField->GetBz(-7.2837383, 10, 151.27782);
  cout << "Field for (-7.2837383, 10, 151.27782) in R3B coordinates or (-10, 10, -10) in field map coordinates: " << B_x << " "  << B_y << " "  << B_z << " "  << endl;
  B_x = 0.1 * gladField->GetBx(12.122176, -10, 156.11626);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(12.122176, -10, 156.11626);
  B_z = 0.1 * gladField->GetBz(12.122176, -10, 156.11626);
  cout << "Field for (12.122176, -10, 156.11626) in R3B coordinates or (10, -10, -10) in field map coordinates: " << B_x << " "  << B_y << " "  << B_z << " "  << endl;
  B_x = 0.1 * gladField->GetBx(12.122176, 10, 156.11626);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(12.122176, 10, 156.11626);
  B_z = 0.1 * gladField->GetBz(12.122176, 10, 156.11626);
  cout << "Field for (12.122176, 10, 156.11626) in R3B coordinates or (10, 10, -10) in field map coordinates: " << B_x << " "  << B_y << " "  << B_z << " "  << endl;


  //CHECKING THE FIELD (COMMENT ME LATER)
  /*B_x = 0.1 * gladField->GetBx(0.0,0.0,163.4);//Field components return in [kG], moved to [T]
  B_y = 0.1 * gladField->GetBy(0.0,0.0,163.4);
  B_z = 0.1 * gladField->GetBz(0.0,0.0,163.4);
  cout << "Field for (25,50,240)" << B_x << " "  << B_y << " "  << B_z << " "  << endl;
*/
  for(Int_t gridPoint_z=1; gridPoint_z<40;gridPoint_z++){ //39 points from 5 to 195 cm (200cm long TPC)
    for(Int_t gridPoint_x= 1; gridPoint_x<16;gridPoint_x++){ //15 points from 5 to 75 cm (80cm wide TPC)
      ele_x_init =
        +cos(-TargetAngle)*(- fHalfSizeTPC_X + gridPoint_x * 5) + sin(-TargetAngle)*(gridPoint_z * 5);
      ele_z_init = (TargetOffsetZ_FM - fHalfSizeTPC_Z)
        -sin(-TargetAngle)*(- fHalfSizeTPC_X + gridPoint_x * 5) + cos(-TargetAngle)*(gridPoint_z * 5);
      if(evtID==0) ele_y_init = -fHalfSizeTPC_Y + 2.5; //2.5cm above the padplane
      else if(evtID==1) ele_y_init = - fHalfSizeTPC_Y + 28.5; //28.5cm above the padplane
      //else if(evtID==2) ele_y_init = TargetOffsetY - fHalfSizeTPC_Y + 15; //15cm above the padplane
      //else if(evtID==3) ele_y_init = TargetOffsetY - fHalfSizeTPC_Y + 20; //20cm above the padplane
      //else if(evtID==4) ele_y_init = TargetOffsetY - fHalfSizeTPC_Y + 25; //25cm above the padplane
      else {
	LOG(INFO) << "Event ID larger than neccesary for the grid test.";
	return;
      }

      //CHECKING THE FIELD (COMMENT ME LATER)
      /*B_x = 0.1 * gladField->GetBx(ele_x_init, ele_y_init, ele_z_init);//Field components return in [kG], moved to [T]
      B_y = 0.1 * gladField->GetBy(ele_x_init, ele_y_init, ele_z_init);
      B_z = 0.1 * gladField->GetBz(ele_x_init, ele_y_init, ele_z_init);
      cout << "Field for (" << ele_x_init << "," << ele_y_init << "," << ele_z_init << "),("<< - fHalfSizeTPC_X+gridPoint_x * 5 <<","<< ele_y_init<< "," << gridPoint_z * 5 <<")" << B_x << " "  << B_y << " "  << B_z << " "  << endl;
     */
      //taken the driftDistance for the calculation of the sigmaLong and sigmaTrans
      driftDistance = ele_y + fHalfSizeTPC_Y;
      sigmaLongAtPadPlane = sqrt(driftDistance*2*fLongDiff/fDriftVelocity);
      sigmaTransvAtPadPlane = sqrt(driftDistance*2*fTransDiff/fDriftVelocity);

      //ele_x = ele_x_init; ele_y = ele_y_init; ele_z = ele_z_init;
      for(Int_t ele=0;ele<generatedElectrons;ele++){
        ele_x = ele_x_init; ele_y = ele_y_init; ele_z = ele_z_init;

        //CHECK... COMMENT ME
        /*cout << "Initial Pos: " << ele_x_init << "," << ele_y_init << "," << ele_z_init << "),("<< - fHalfSizeTPC_X+gridPoint_x * 5 <<","<< ele_y_init<< "," << gridPoint_z * 5 <<")" << endl;
*/
	accDriftTime = timeBeforeDrift;
	driftTimeStep = 100; // 100ns TODO set as variable or move to parameter container

	LOG(DEBUG) << "R3BGTPCLangevinTest::Exec, INITIAL VALUES: timeBeforeDrift="<< accDriftTime << " [ns]"
		   << " ele_x=" << ele_x <<" ele_y=" << ele_y <<" ele_z=" << ele_z << " [cm]";

	//cout <<  "R3BGTPCLangevinTest::Exec, INITIAL VALUES: timeBeforeDrift="<< accDriftTime << " [ns]"
	//     << " ele_x=" << ele_x <<" ele_y=" << ele_y <<" ele_z=" << ele_z << " [cm]" << endl;


	while(ele_y>-fHalfSizeTPC_Y){ //while not reaching the pad plane [cm]
	  B_x = 0.1 * gladField->GetBx(ele_x, ele_y, ele_z);//Field components return in [kG], moved to [T]
	  B_y = 0.1 * gladField->GetBy(ele_x, ele_y, ele_z);
	  B_z = 0.1 * gladField->GetBz(ele_x, ele_y, ele_z);

    //if(ele==0) cout << "Field for (" << ele_x << "," << ele_y << "," << ele_z << ")" << B_x << " "  << B_y << " "  << B_z << " "  << endl;

    moduleB = TMath::Sqrt(B_x*B_x+B_y*B_y+B_z*B_z);  //in [T]
	  cteMod = 1/(1+ mu*mu*moduleB*moduleB);  //SI
	  cteMult = mu*cteMod;  //SI

	  //assuming only vertical electric field in the next four lines
	  productEB = E_y*B_y; //E_x*B_x + E_y*B_y + E_z*B_z; SI

	  vDrift_x = cteMult * (mu*(E_y*B_z) + mu*mu*productEB*B_x); //cte * (Ex + mu*(E_y*B_z-E_z*B_y) + mu*mu*productEB*B_x); SI
	  vDrift_y = cteMult * (E_y + mu*mu*productEB*B_y);          //cte * (Ey + mu*(E_z*B_x-E_x*B_z) + mu*mu*productEB*B_y); SI
	  vDrift_z = cteMult * (mu*(-E_y*B_x) + mu*mu*productEB*B_z);//cte * (Ez + mu*(E_x*B_y-E_y*B_x) + mu*mu*productEB*B_z); SI

	  LOG(DEBUG) << "R3BGTPCLangevinTest::Exec, timeBeforeDrift=vDrift_x=" << vDrift_x
		     <<" vDrift_y=" << vDrift_y <<" vDrift_z=" << vDrift_z <<  " [m/s]";

	  //adjusting the last step before the pad plane
	  if(ele_y-1.E-7*vDrift_y*driftTimeStep<-fHalfSizeTPC_Y)
	    driftTimeStep = (ele_y+fHalfSizeTPC_Y)/(1.E-7*vDrift_y); //vDrift back to [cm/ns]

	  //reducing sigmaTransv (see http://web.ift.uib.no/~lipniack/detectors/lecture5/detector5.pdf)
	  //as B~B_y and E=E_y, let's simplify and apply the reduction to the transversal coefficient without projections
	  sigmaTransvStep = sqrt(driftTimeStep*2*fTransDiff*cteMod); //should be reduced by the factor cteMod=cteMult/mu
	  sigmaLongStep = sqrt(driftTimeStep*2*fLongDiff);  //should be the same scaled to the length of the step
	  ele_x = gRandom->Gaus(ele_x+1.E-7*vDrift_x*driftTimeStep,sigmaTransvStep);//vDrift back to [cm/ns]
	  ele_y = gRandom->Gaus(ele_y-1.E-7*vDrift_y*driftTimeStep,sigmaLongStep);
	  ele_z = gRandom->Gaus(ele_z+1.E-7*vDrift_z*driftTimeStep,sigmaTransvStep);
	  accDriftTime = accDriftTime + driftTimeStep;

	  LOG(DEBUG) << "R3BGTPCLangevinTest::Exec, accDriftTime="<< accDriftTime << " [ns]"
		     << " ele_x=" << ele_x <<" ele_y=" << ele_y <<" ele_z=" << ele_z << " [cm]";


	}
	//cout << "R3BGTPCLangevinTest::Exec, accDriftTime="<< accDriftTime << " [ns]"
	//     << " ele_x=" << ele_x <<" ele_y=" << ele_y <<" ele_z=" << ele_z << " [cm]" <<endl;

	//projX = ele_x;
	//projZ = ele_z;
	projTime = accDriftTime;

	//obtain padID for projX, projZ (simple algorithm)
	//the algorithm assigns a pad number which depends on the XZ size of the chamber,
	//according to the fSizeOfVirtualPad parameter: if it is 1, the pad size is cm^2
	//and padID goes from 0 to 2*fHalfSizeTPC_X in the first row,
	//from 2*fHalfSizeTPC_X to 4*fHalfSizeTPC_X in the second row, ...
	//if fSizeOfVirtualPad=0.1, then padID goes from 0 to 20*fHalfSizeTPC_X,
	//from 20*fHalfSizeTPC_X to 40*fHalfSizeTPC_X, ...
	//Avoid first moving out of the virtual pad plane limits

  projX =
    +cos(TargetAngle)*ele_x + sin(TargetAngle)*(ele_z-(TargetOffsetZ_FM-fHalfSizeTPC_Z));
  projZ=
    -sin(TargetAngle)*ele_x + cos(TargetAngle)*(ele_z-(TargetOffsetZ_FM-fHalfSizeTPC_Z));

    //CHECK... COMMENT ME
    /*cout << "Final Pos: " << ele_x<< "," << ele_y<< "," << ele_z << "),("<< projX <<","<< -fHalfSizeTPC_Y << "," << projZ <<")" << endl;
*/
  //if(projZ<TargetOffsetZ-fHalfSizeTPC_Z) projZ=TargetOffsetZ-fHalfSizeTPC_Z;
	//if(projZ>TargetOffsetZ+fHalfSizeTPC_Z) projZ=TargetOffsetZ+fHalfSizeTPC_Z;
	//if(projX<TargetOffsetX-fHalfSizeTPC_X) projX=TargetOffsetX-fHalfSizeTPC_X;
	//if(projX>TargetOffsetX+fHalfSizeTPC_X) projX=TargetOffsetX+fHalfSizeTPC_X;

	Int_t padID = (2*fHalfSizeTPC_X*fSizeOfVirtualPad) *
	  (Int_t)((projZ)*fSizeOfVirtualPad) +
	  (Int_t)((projX-fHalfSizeTPC_X)*fSizeOfVirtualPad);
    //cout << "padID: " << padID << endl;

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
    }
  }
  LOG(INFO) << "R3BGTPCLangevinTest: produced " << fGTPCProjPoint->GetEntries() << " projPoints";
}

void R3BGTPCLangevinTest::Finish() {

}

ClassImp(R3BGTPCLangevinTest)
