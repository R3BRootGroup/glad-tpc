#include <iomanip>
#include <iostream>
#include "TGeoManager.h"
#include "TMath.h"
#include "TFile.h"
#include "TGeoBBox.h"

TGeoRotation *zeroRot = new TGeoRotation;
TGeoTranslation *zeroTrans = new TGeoTranslation;
TGeoCombiTrans *zeroRotTrans = new TGeoCombiTrans(*zeroTrans, *zeroRot);

// Create a null translation
TGeoTranslation *fGlobalTrans = new TGeoTranslation();
// Create Matrix Unity
TGeoRotation *fGlobalRot = new TGeoRotation();
TGeoRotation *fRefRot = NULL;

TGeoManager*  gGeoMan = NULL;

Double_t fThetaX = 0.;
Double_t fThetaY = 0.;
Double_t fThetaZ = 0.;
Double_t fPhi   = 0.;
Double_t fTheta = 0.;
Double_t fPsi   = 0.;
Double_t fX = 0.;
Double_t fY = 0.;
Double_t fZ = 0.;
Bool_t fLocalTrans = kFALSE;
Bool_t fLabTrans = kTRUE;

//________________________GTPC parameters
Double_t TargetLength;
Double_t TargetRadius;
Double_t WindowThickness;
Double_t ChamberLength;
Double_t ChamberInnerRadius;
Double_t TPCRadiusExt;
Double_t TPCLx;
Double_t TPCLy;
Double_t TPCLz;
Double_t ChamberThickness;
Double_t InnerRohacellThickness;
Double_t OuterRohacellThickness;
Double_t KaptonThickness;
Double_t TargetAngle;
Double_t TargetOffsetX;
Double_t TargetOffsetY;
Double_t TargetOffsetZ;

Double_t WorldSizeX;
Double_t WorldSizeY;
Double_t WorldSizeZ;

void ConstructTPC(TGeoVolume* pWorld);

string GEOTAG;

void create_tpc_geo(const char* geoTag="v1") {
  GEOTAG = std::string(geoTag);
  fGlobalTrans->SetTranslation(0.0,0.0,0.0);

  // -------   Load media from media file   -----------------------------------
  FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString medFile = geoPath + "/glad-tpc/geometry/media_tpc.geo";
  geoFace->setMediaFile(medFile);
  geoFace->readMedia();
  gGeoMan = gGeoManager;
  // --------------------------------------------------------------------------
  
  // -------   Geometry file name (output)   ----------------------------------
  TString geoFileName = geoPath + "/glad-tpc/geometry/gladTPC_";
  geoFileName = geoFileName + geoTag + ".geo.root";
  // --------------------------------------------------------------------------
  
  // -----------------   Get and create the required media    -----------------
  FairGeoMedia*   geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();
  
  //geoMedia->print();
  //
  FairGeoMedium* mvacuum      = geoMedia->getMedium("vacuum");
  if ( ! mvacuum ) Fatal("Main", "FairMedium vacuum not found");
  geoBuild->createMedium(mvacuum);
  TGeoMedium* pvacuum = gGeoMan->GetMedium("vacuum");
  if ( ! pvacuum ) Fatal("Main", "Medium vacuum not found");
  
  FairGeoMedium* mair      = geoMedia->getMedium("air");
  if ( ! mair ) Fatal("Main", "FairMedium air not found");
  geoBuild->createMedium(mair);
  TGeoMedium* pair = gGeoMan->GetMedium("air");
  if ( ! pair ) Fatal("Main", "Medium Air not found");
  
  FairGeoMedium* mLH2      = geoMedia->getMedium("LH2");
  if ( ! mLH2 ) Fatal("Main", "FairMedium LH2 not found");
  geoBuild->createMedium(mLH2);
  TGeoMedium* pLH2 = gGeoMan->GetMedium("LH2");
  if ( ! pLH2 ) Fatal("Main", "Medium LH2 not found");
  
  FairGeoMedium* mcarbon      = geoMedia->getMedium("carbon");
  if ( ! mcarbon ) Fatal("Main", "FairMedium carbon not found");
  geoBuild->createMedium(mcarbon);
  TGeoMedium* pcarbon = gGeoMan->GetMedium("carbon");
  if ( ! pcarbon ) Fatal("Main", "Medium carbon not found");
  
  FairGeoMedium* mAl      = geoMedia->getMedium("mylar");
  if ( ! mAl ) Fatal("Main", "FairMedium mylar not found");
  geoBuild->createMedium(mAl);
  TGeoMedium* pAl = gGeoMan->GetMedium("mylar");
  if ( ! pAl ) Fatal("Main", "Medium mylar not found");
  
  FairGeoMedium* minox      = geoMedia->getMedium("inox");
  if ( ! minox ) Fatal("Main", "FairMedium inox not found");
  geoBuild->createMedium(minox);
  TGeoMedium* pinox = gGeoMan->GetMedium("inox");
  if ( ! pinox ) Fatal("Main", "Medium inox not found");
  
  FairGeoMedium* mrohacell      = geoMedia->getMedium("rohacell");
  if ( ! mrohacell ) Fatal("Main", "FairMedium rohacell not found");
  geoBuild->createMedium(mrohacell);
  TGeoMedium* prohacell = gGeoMan->GetMedium("rohacell");
  if ( ! prohacell ) Fatal("Main", "Medium rohacell not found");
  
  FairGeoMedium* mkapton      = geoMedia->getMedium("kapton");
  if ( ! mkapton ) Fatal("Main", "FairMedium kapton not found");
  geoBuild->createMedium(mkapton);
  TGeoMedium* pkapton = gGeoMan->GetMedium("kapton");
  if ( ! pkapton ) Fatal("Main", "Medium kapton not found");
  
  FairGeoMedium* mmix      = geoMedia->getMedium("mix");
  if ( ! mmix ) Fatal("Main", "FairMedium mix not found");
  geoBuild->createMedium(mmix);
  TGeoMedium* pmix = gGeoMan->GetMedium("mix");
  if ( ! pmix ) Fatal("Main", "Medium mix not found");
  //_________________________________
  
  // --------------   Create geometry and top volume  -------------------------
  gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
  gGeoMan->SetName("GTPCgeom");
  TGeoVolume* top = new TGeoVolumeAssembly("TOP");
  gGeoMan->SetTopVolume(top);
  
  //top->SetVisLeaves(kTRUE);
  //gGeoManager->SetVisLevel(4);
  //gGeoManager->SetVisOption(0);
  // --------------------------------------------------------------------------
  
  R3BGTPCSetup* setup = new R3BGTPCSetup(1);
  setup->Print();
  
  TargetLength           = setup->GetTargetLength()/2.;//cm
  TargetRadius           = setup->GetTargetRadius();
  WindowThickness        = setup->GetWindowThickness()/2.;
  ChamberLength          = setup->GetChamberLength()/2.;
  ChamberInnerRadius     = setup->GetChamberInnerRadius();
  TPCRadiusExt           = setup->GetTPCRadiusExt();
  TPCLx                  = setup->GetTPCLx()/2.;
  TPCLy                  = setup->GetTPCLy()/2.;
  TPCLz                  = setup->GetTPCLz()/2.;
  ChamberThickness       = setup->GetChamberThickness();
  InnerRohacellThickness = setup->GetInnerRohacellThickness();
  OuterRohacellThickness = setup->GetOuterRohacellThickness();
  KaptonThickness        = setup->GetKaptonThickness();
  TargetAngle            = setup->GetTargetAngle();//deg
  TargetOffsetX          = setup->GetTargetOffsetX();//cm
  TargetOffsetY          = setup->GetTargetOffsetY();//cm
  TargetOffsetZ          = setup->GetTargetOffsetZ();//cm
  delete setup;

  //
  /*
  TargetLength           = 15;//cm
  TargetRadius           = 2.8;
  WindowThickness        = 0.0150;
  ChamberLength          = 20;
  ChamberInnerRadius     = 3.7;
  TPCRadiusExt           = 10;
  TPCLx                  = 50;
  TPCLy                  = 20;
  TPCLz                  = 100;
  ChamberThickness       = 0.1;
  InnerRohacellThickness = 0.1;
  OuterRohacellThickness = 0.2;
  KaptonThickness        = 0.0125;
  TargetAngle            = 0.;//deg
  TargetOffsetX          = 0.;//cm
  TargetOffsetY          = 0.;//cm
  TargetOffsetZ          = 113.4;//cm
  */

  WorldSizeX            = 1.2*2*TPCLx;
  WorldSizeY            = 1.2*2*TPCLy;
  WorldSizeZ            = 1.2*2*TPCLz;
  

  //DELETE ME! TO avoid collision with previos parameters, just a TEST
  TargetOffsetZ = 200;

  // Geometry description
  //TGeoVolume *pAWorld =  gGeoManager->GetTopVolume();
  
  // GTPC main area
  TGeoVolume *pAWorld  =  gGeoManager->GetTopVolume();

  TGeoShape *tpc_box = new TGeoBBox("GTPC_Box",
				    WorldSizeX,
				    WorldSizeY,
				    WorldSizeZ);
  TGeoVolume* pWorld = new TGeoVolume("GTPC_box",tpc_box, pvacuum);

  //pAWorld->AddNodeOverlap(pWorld, 0, zeroRotTrans);


  //__________________________
  ConstructTPC(pWorld);
  
  //_____________________________Globle definition of TPC position in cm
  fGlobalRot->RotateY(TargetAngle);//deg
  TVector3 gTrans(TargetOffsetX, TargetOffsetY, TargetOffsetZ);//offset before ratation
  gTrans.RotateY(TargetAngle*TMath::DegToRad());
  
  fGlobalTrans->SetTranslation(gTrans.X(),gTrans.Y(),gTrans.Z());
  
  TGeoCombiTrans *pGlobal = new TGeoCombiTrans(*fGlobalTrans,*fGlobalRot);
  //pAWorld->AddNode(pWorld, 0, pGlobal);
  top->AddNode(pWorld, 0, pGlobal);
  
  //____________________________________
  gGeoMan->CloseGeometry();
  gGeoMan->CheckOverlaps(0.001);
  gGeoMan->PrintOverlaps();
  gGeoMan->Test();
  
  cout<<"vis option "<<gGeoMan->GetVisOption()<<" =====  vis level "<<gGeoMan->GetVisLevel()<<endl;
  
  
  TFile* geoFile = new TFile(geoFileName, "RECREATE");
  top->Draw("ogl");
  top->Write();
  geoFile->Close();
}
//________________________________________________________________
void ConstructTPC(TGeoVolume* pWorld) {
  
  TGeoMedium *TargetMaterial = gGeoManager->GetMedium("LH2");
  TGeoMedium *ChamberMaterial = gGeoManager->GetMedium("inox");
  TGeoMedium *TPCMaterial = gGeoManager->GetMedium("mix");
  TGeoMedium *GapMaterial = gGeoManager->GetMedium("air");
  TGeoMedium *WindowMaterial = gGeoManager->GetMedium("mylar");
  TGeoMedium *KaptonMaterial = gGeoManager->GetMedium("kapton");
  TGeoMedium *InnerRohacellMaterial = gGeoManager->GetMedium("rohacell");
  TGeoMedium *OuterRohacellMaterial = gGeoManager->GetMedium("rohacell");
  
  //
  TGeoShape* solidTarget=0;  TGeoVolume* logicTarget=0;
  TGeoShape* solidChamber=0; TGeoVolume* logicChamber=0;
  TGeoShape* solidTPC=0;     TGeoVolume* logicTPC=0;
  TGeoShape* solidKapton = 0;TGeoVolume* logicKapton=0;
  TGeoShape* solidInnerRohacell = 0; TGeoVolume* logicInnerRohacell=0;
  TGeoShape* solidOuterRohacell = 0; TGeoVolume* logicOuterRohacell=0;
  TGeoShape* solidWindow0 = 0; TGeoVolume* logicWindow0=0;
  TGeoShape* solidWindow2 = 0; TGeoVolume* logicWindow1=0;
  TGeoShape* solidWindow1 = 0; TGeoVolume* logicWindow2=0;
  TGeoShape* solidGap=0;     TGeoVolume* logicGap=0;
  
  //___________Target
  if(GEOTAG=="v1") {
    solidTarget = new TGeoTubeSeg("Target", 0.,TargetRadius,TargetLength,0,360.);
    logicTarget = new TGeoVolume("Target",solidTarget,TargetMaterial);
  }
  else if(GEOTAG=="v2") {
    TargetLength = 4.0/2;
    WindowThickness = 0.0001;//no window
    TargetMaterial = gGeoManager->GetMedium("carbon");
    solidTarget = new TGeoTubeSeg("Target", 0.,TargetRadius,TargetLength, 0,360.);
    logicTarget = new TGeoVolume("Target",solidTarget,TargetMaterial);
  }
  
  pWorld->AddNode(logicTarget, 0, new TGeoCombiTrans(0.,0.,TargetLength,zeroRot));
  
  //___________Chamber
  //solidChamber = new TGeoTubeSeg  ("Chamber",ChamberInnerRadius,ChamberInnerRadius+ChamberThickness,ChamberLength,0,360.);
  TGeoShape* chamber_box1 = new TGeoBBox("chamber_box1",
					 ChamberInnerRadius,
					 ChamberInnerRadius,
					 ChamberLength);
  TGeoShape* chamber_box2 = new TGeoBBox("chamber_box2",
					 ChamberInnerRadius+ChamberThickness,
					 ChamberInnerRadius+ChamberThickness,
					 ChamberLength);
  
  TGeoTranslation *tc = new TGeoTranslation("tc",0,0,-1*ChamberThickness);
  tc->RegisterYourself();
  solidChamber = new TGeoCompositeShape("Chamber", "chamber_box2 - chamber_box1:tc");
  
  logicChamber = new TGeoVolume("Chamber",solidChamber,ChamberMaterial);
  pWorld->AddNode(logicChamber, 0, new TGeoCombiTrans(0.,0.,ChamberLength,zeroRot));
  
  //_______________________ Inner Rohacell
  //solidInnerRohacell = new TGeoTubeSeg("InnerRohacell",
  //                     ChamberInnerRadius + ChamberThickness,
  //                     ChamberInnerRadius + ChamberThickness+InnerRohacellThickness,
  //                     ChamberLength,0,360.);
  
  TGeoShape* innerRohacell_box1 = 
    new TGeoBBox("innerRohacell_box1",
		 ChamberInnerRadius+ChamberThickness,
		 ChamberInnerRadius+ChamberThickness,
		 ChamberLength+InnerRohacellThickness*0.5);//================================================
  TGeoShape* innerRohacell_box2 = 
    new TGeoBBox("innerRohacell_box2",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness,
		 ChamberLength+InnerRohacellThickness*0.5);
  TGeoTranslation *ti = new TGeoTranslation("ti",0,0,-1*InnerRohacellThickness);
  ti->RegisterYourself();
  
  solidInnerRohacell = new TGeoCompositeShape("InnerRohacell", 
					      "innerRohacell_box2 - innerRohacell_box1:ti");
  
  logicInnerRohacell = new TGeoVolume("InnerRohacell",solidInnerRohacell, InnerRohacellMaterial);
  pWorld->AddNode(logicInnerRohacell, 0, new TGeoCombiTrans(0.,0.,ChamberLength+InnerRohacellThickness*0.5,zeroRot));
  
  //_________Outer Rohacell
  //solidOuterRohacell = new TGeoTubeSeg("OuterRohacell",
  //                     ChamberInnerRadius + ChamberThickness + InnerRohacellThickness + KaptonThickness,
  //                     ChamberInnerRadius + ChamberThickness + InnerRohacellThickness + KaptonThickness+OuterRohacellThickness,
  //                     ChamberLength,0,360.);
  
  TGeoShape* outerRohacell_box1 = 
    new TGeoBBox("outerRohacell_box1",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness,
		 ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5+OuterRohacellThickness*0.5);
  TGeoShape* outerRohacell_box2 = 
    new TGeoBBox("outerRohacell_box2",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness+OuterRohacellThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness+OuterRohacellThickness,
		 ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5+OuterRohacellThickness*0.5);
  TGeoTranslation *to = new TGeoTranslation("to",0,0,-1*OuterRohacellThickness);
  to->RegisterYourself();
  solidOuterRohacell = new TGeoCompositeShape("OuterRohacell", "outerRohacell_box2 - outerRohacell_box1:to");
  
  logicOuterRohacell = new TGeoVolume("OuterRohacell",solidOuterRohacell,OuterRohacellMaterial);
  pWorld->AddNode(logicOuterRohacell, 0, new TGeoCombiTrans(0.,0.,ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5+OuterRohacellThickness*0.5,zeroRot));
  
  //_________________Kapton
  //solidKapton = new TGeoTubeSeg("Kapton",
  //              ChamberInnerRadius + ChamberThickness+InnerRohacellThickness,
  //              ChamberInnerRadius + ChamberThickness+InnerRohacellThickness+KaptonThickness,
  //              ChamberLength,0,360.);
  //
  TGeoShape* kapton_box1 = 
    new TGeoBBox("kapton_box1",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness,
		 ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5);
  TGeoShape* kapton_box2 = 
    new TGeoBBox("kapton_box2",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness,
		 ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5);
  TGeoTranslation *tk = new TGeoTranslation("tk",0,0,-1*KaptonThickness);
  tk->RegisterYourself();
  solidKapton = new TGeoCompositeShape("Kapton", "kapton_box2 - kapton_box1:tk");
  
  logicKapton = new TGeoVolume("Kapton",solidKapton,KaptonMaterial);
  pWorld->AddNode(logicKapton, 0, new TGeoCombiTrans(0.,0.,ChamberLength+InnerRohacellThickness*0.5+KaptonThickness*0.5,zeroRot));
  
  //________________TPC
  //solidTPC = new TGeoTubeSeg("TPC",
  //           ChamberInnerRadius + ChamberThickness + InnerRohacellThickness + KaptonThickness + OuterRohacellThickness,
  //           TPCRadiusExt,
  //           ChamberLength,0,360.);
  //
  TGeoShape* tpc_box1 = 
    new TGeoBBox("tpc_box1",
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness+OuterRohacellThickness,
		 ChamberInnerRadius+ChamberThickness+InnerRohacellThickness+KaptonThickness+OuterRohacellThickness,
		 ChamberLength+ChamberThickness*0.5+InnerRohacellThickness*0.5+KaptonThickness*0.5+OuterRohacellThickness*0.5+0.1);
  cout<<"TPCLz ChamberLength "<<TPCLz<<"  "<<ChamberLength<<endl;
  TGeoTranslation *t1 = new TGeoTranslation("t1",0,0,-1*(TPCLz-(ChamberLength+ChamberThickness*0.5+InnerRohacellThickness*0.5+KaptonThickness*0.5+OuterRohacellThickness*0.5+0.1))-0.1);
  t1->RegisterYourself();
  TGeoShape* tpc_box2 = new TGeoBBox("tpc_box2",TPCLx, TPCLy,  TPCLz);
  solidTPC = new TGeoCompositeShape("GTPCGas", "tpc_box2 - tpc_box1:t1");
  
  logicTPC = new TGeoVolume("GTPCGas",solidTPC,TPCMaterial);
  pWorld->AddNode(logicTPC, 0, new TGeoCombiTrans(0.,0.,TPCLz,zeroRot));
  
  //______________window
  solidWindow0 = new TGeoTubeSeg("WindowTube",TargetRadius,TargetRadius+WindowThickness*2.,TargetLength,0,360.);
  logicWindow0 = new TGeoVolume("WindowTube",solidWindow0,WindowMaterial);
  pWorld->AddNode(logicWindow0, 0, new TGeoCombiTrans(0.,0.,TargetLength,zeroRot));
  
  solidWindow1 = new TGeoTubeSeg("WindowEntrance", 0.,TargetRadius+2.*WindowThickness,WindowThickness,0,360.);
  logicWindow1 = new TGeoVolume("WindowEntrance",solidWindow1,WindowMaterial);
  pWorld->AddNode(logicWindow1, 0, new TGeoCombiTrans(0.,0.,-1*WindowThickness,zeroRot));
  
  solidWindow2 = new TGeoTubeSeg("WindowOutcoming",0.,TargetRadius+2.*WindowThickness,WindowThickness,0,360.);
  logicWindow2 = new TGeoVolume("WindowOutcoming",solidWindow2,WindowMaterial);
  pWorld->AddNode(logicWindow2, 0, new TGeoCombiTrans(0.,0.,2*TargetLength+WindowThickness,zeroRot));
  
  //__________________gap
  TGeoShape* gap_box1 = new TGeoTubeSeg("gap_box1", 0, TargetRadius+WindowThickness*2, TargetLength+2*WindowThickness, 0, 360.);
  TGeoTranslation *tg = new TGeoTranslation("tg",0,0,-1*(ChamberLength-0.5*ChamberThickness-(TargetLength+2*WindowThickness))-2*WindowThickness);
  tg->RegisterYourself();
  TGeoShape* gap_box2 = new TGeoBBox("gap_box2",ChamberInnerRadius, ChamberInnerRadius, ChamberLength-0.5*ChamberThickness);
  solidGap = new TGeoCompositeShape("gap", "gap_box2 - gap_box1:tg");
  
  logicGap = new TGeoVolume("Gap",solidGap,GapMaterial);
  pWorld->AddNode(logicGap, 0, new TGeoCombiTrans(0.,0.,ChamberLength-0.5*ChamberThickness,zeroRot));
  
  logicTarget->SetLineColor(kAzure);
  logicInnerRohacell->SetLineColor(kOrange);
  logicOuterRohacell->SetLineColor(kOrange);
  logicKapton->SetLineColor(kAzure);
  logicChamber->SetLineColor(kSpring);
  logicTPC->SetLineColor(kSpring);
  logicGap->SetLineColor(kWhite); 
}
