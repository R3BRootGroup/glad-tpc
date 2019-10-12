#include <iomanip>
#include <iostream>
#include "TGeoManager.h"
#include "TMath.h"
#include "TFile.h"
#include "TGeoBBox.h"

TGeoRotation *zeroRot = new TGeoRotation;

// Create a null translation
TGeoTranslation *fGlobalTrans = new TGeoTranslation();
// Create Matrix Unity
TGeoRotation *fGlobalRot = new TGeoRotation();

TGeoManager*  gGeoMan = NULL;
//________________________GTPC parameters
Double_t TPCLx = 40; //halfsizes
Double_t TPCLy = 15;
Double_t TPCLz = 100;
Double_t TargetAngle=0;
Double_t TargetOffsetX=-25;
Double_t TargetOffsetY=0;
Double_t TargetOffsetZ=300;

string GEOTAG;

void create_tpc_geo_test(const char* geoTag="test") {
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
  
  FairGeoMedium* themix      = geoMedia->getMedium("mix");
  if ( ! themix ) Fatal("Main", "FairMedium mix not found");
  geoBuild->createMedium(themix);
  TGeoMedium* pthemix = gGeoMan->GetMedium("mix");
  if ( ! pthemix ) Fatal("Main", "Medium Mix not found");
  
  
  // --------------   Create geometry and top volume  -------------------------
  gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
  gGeoMan->SetName("GTPCgeom");
  TGeoVolume* top = new TGeoVolumeAssembly("TOP");
  gGeoMan->SetTopVolume(top);
  
  //top->SetVisLeaves(kTRUE);
  //gGeoManager->SetVisLevel(4);
  //gGeoManager->SetVisOption(0);
  // --------------------------------------------------------------------------
 
  Double_t WorldSizeX            = 1.01*TPCLx;
  Double_t WorldSizeY            = 1.01*TPCLy;
  Double_t WorldSizeZ            = 1.01*TPCLz;

  // GTPC main area
  TGeoVolume *pAWorld  =  gGeoManager->GetTopVolume();

  TGeoShape *tpc_box = new TGeoBBox("GTPC_Box",
				    WorldSizeX,
				    WorldSizeY,
				    WorldSizeZ);
  TGeoVolume* pWorld = new TGeoVolume("GTPC_box",tpc_box, pvacuum);

  //_________________________TPC
  TGeoShape* solidTPC=0;     TGeoVolume* logicTPC=0;
  solidTPC = new TGeoBBox("tpc_box",TPCLx, TPCLy,  TPCLz);
  logicTPC = new TGeoVolume("GTPCGas",solidTPC,pthemix);
  pWorld->AddNode(logicTPC, 0, new TGeoCombiTrans(0.,0.,0.,zeroRot));
  
  logicTPC->SetLineColor(kSpring);
  
  //_____________________________Globle definition of TPC position in cm
  fGlobalRot->RotateY(TargetAngle);//deg
  TVector3 gTrans(TargetOffsetX, TargetOffsetY, TargetOffsetZ);//offset before ratation
  gTrans.RotateY(TargetAngle*TMath::DegToRad());
  
  fGlobalTrans->SetTranslation(gTrans.X(),gTrans.Y(),gTrans.Z());
  
  TGeoCombiTrans *pGlobal = new TGeoCombiTrans(*fGlobalTrans,*fGlobalRot);
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
