#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TMath.h"
#include <iomanip>
#include <iostream>

TGeoRotation* zeroRot = new TGeoRotation;
TGeoTranslation* zeroTrans = new TGeoTranslation;
TGeoCombiTrans* zeroRotTrans = new TGeoCombiTrans(*zeroTrans, *zeroRot);

// Create a null translation
TGeoTranslation* fGlobalTrans = new TGeoTranslation();
// Create Matrix Unity
TGeoRotation* fGlobalRot = new TGeoRotation();
TGeoRotation* fRefRot = NULL;

TGeoManager* gGeoMan = NULL;

Double_t fThetaX = 0., fThetaY = 0., fThetaZ = 0.;
Double_t fPhi = 0., fTheta = 0., fPsi = 0.;
Double_t fX = 0., fY = 0., fZ = 0.;
Bool_t fLocalTrans = kFALSE;
Bool_t fLabTrans = kTRUE;

//________________________HYDRA parameters
Double_t TargetLength;
Double_t TargetRadius;
Double_t TargetAngle;
Double_t TargetOffsetX;
Double_t TargetOffsetY;
Double_t TargetOffsetZ;

Double_t TPCLx;
Double_t TPCLy;
Double_t TPCLz;
Double_t FrameThickness;
Double_t ActiveRegionx;
Double_t ActiveRegiony;
Double_t ActiveRegionz;
Double_t Windowx;
Double_t Windowy;
Double_t Windowz;

Double_t WorldSizeX;
Double_t WorldSizeY;
Double_t WorldSizeZ;

void ConstructTPC(TGeoVolume* pWorld);

string GEOTAG;

void create_tpc_geo(const char* geoTag = "Prototype")
{ // Putting the correct GEOTAG is essential
    GEOTAG = std::string(geoTag);
    fGlobalTrans->SetTranslation(0.0, 0.0, 0.0);

    // -------   Load media from media file   -----------------------------------
    FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();
    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    TString medFile = geoPath + "/glad-tpc/geometry/media_tpc.geo";
    geoFace->setMediaFile(medFile);
    geoFace->readMedia();
    gGeoMan = gGeoManager;
    // --------------------------------------------------------------------------

    // -------   Geometry file name (output)   ----------------------------------
    TString geoFileName = geoPath + "/glad-tpc/geometry/HYDRA_";
    geoFileName = geoFileName + geoTag + ".geo.root";
    // --------------------------------------------------------------------------

    // -----------------   Get and create the required media    -----------------
    FairGeoMedia* geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

    // geoMedia->print();

    // Defined by Simone----------------------------------------------------------
    FairGeoMedium* mMy = geoMedia->getMedium("mylar");
    if (!mMy)
        Fatal("Main", "FairMedium mylar not found");
    geoBuild->createMedium(mMy);
    TGeoMedium* pMy = gGeoMan->GetMedium("mylar");
    if (!pMy)
        Fatal("Main", "Medium mylar not found");

    FairGeoMedium* mAl = geoMedia->getMedium("aluminium");
    if (!mAl)
        Fatal("Main", "FairMedium aluminium not found");
    geoBuild->createMedium(mAl);
    TGeoMedium* pAl = gGeoMan->GetMedium("aluminium");
    if (!pAl)
        Fatal("Main", "Medium aluminium not found");
    // this is the T2K gas mixture
    FairGeoMedium* mmix = geoMedia->getMedium("mix");
    if (!mmix)
        Fatal("Main", "FairMedium mix not found");
    geoBuild->createMedium(mmix);
    TGeoMedium* pmix = gGeoMan->GetMedium("mix");
    if (!pmix)
        Fatal("Main", "Medium mix not found");
    // --------------------------------------------------------------------------

    FairGeoMedium* mvacuum = geoMedia->getMedium("vacuum");
    if (!mvacuum)
        Fatal("Main", "FairMedium vacuum not found");
    geoBuild->createMedium(mvacuum);
    TGeoMedium* pvacuum = gGeoMan->GetMedium("vacuum");
    if (!pvacuum)
        Fatal("Main", "Medium vacuum not found");

    FairGeoMedium* mair = geoMedia->getMedium("Air");
    if (!mair)
        Fatal("Main", "FairMedium Air not found");
    geoBuild->createMedium(mair);
    TGeoMedium* pair = gGeoMan->GetMedium("Air");
    if (!pair)
        Fatal("Main", "Medium Air not found");

    FairGeoMedium* mLH2 = geoMedia->getMedium("LH2");
    if (!mLH2)
        Fatal("Main", "FairMedium LH2 not found");
    geoBuild->createMedium(mLH2);
    TGeoMedium* pLH2 = gGeoMan->GetMedium("LH2");
    if (!pLH2)
        Fatal("Main", "Medium LH2 not found");

    FairGeoMedium* mkapton = geoMedia->getMedium("kapton");
    if (!mkapton)
        Fatal("Main", "FairMedium kapton not found");
    geoBuild->createMedium(mkapton);
    TGeoMedium* pkapton = gGeoMan->GetMedium("kapton");
    if (!pkapton)
        Fatal("Main", "Medium kapton not found");

    FairGeoMedium* mcarbon = geoMedia->getMedium("carbon");
    if (!mcarbon)
        Fatal("Main", "FairMedium carbon not found");
    geoBuild->createMedium(mcarbon);
    TGeoMedium* pcarbon = gGeoMan->GetMedium("carbon");
    if (!pcarbon)
        Fatal("Main", "Medium carbon not found");
    //_________________________________

    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
    gGeoMan->SetName("GTPCgeom");
    TGeoVolume* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);

    top->SetVisLeaves(kTRUE);
    gGeoManager->SetVisLevel(4);
    gGeoManager->SetVisOption(0);
    // --------------------------------------------------------------------------

    cout << "\n \033[1;31m Warning\033[0m: the detector you are building is " << geoTag << "!!!!!\n" << endl;
    R3BGTPCSetup* setup = new R3BGTPCSetup(geoTag, 0);
    setup->Print();

    TargetLength = setup->GetTargetLength() / 2.; // cm
    TargetRadius = setup->GetTargetRadius();      // cm
    TargetAngle = setup->GetTargetAngle();        // deg
    TargetOffsetX = setup->GetTargetOffsetX();    // cm
    TargetOffsetY = setup->GetTargetOffsetY();    // cm
    TargetOffsetZ = setup->GetTargetOffsetZ();    // cm

    ActiveRegionx = setup->GetActiveRegionx() / 2.; // cm
    ActiveRegiony = setup->GetActiveRegiony() / 2.; // cm
    ActiveRegionz = setup->GetActiveRegionz() / 2.; // cm

    TPCLx = setup->GetTPCLx() / 2.;                   // cm
    TPCLy = setup->GetTPCLy() / 2.;                   // cm
    TPCLz = setup->GetTPCLz() / 2.;                   // cm
    FrameThickness = setup->GetFrameThickness() / 2.; // cm

    Windowx = setup->GetWindowx() / 2.; // cm
    Windowy = setup->GetWindowy() / 2.; // cm
    Windowz = setup->GetWindowz() / 2.; // cm

    delete setup;

    /*
    TargetLength           = 5;															//cm
    TargetRadius           = 0.25;													//cm
    TargetAngle            = 0.;														//deg
    TargetOffsetX          = -10.2.;												//cm
    TargetOffsetY          = 0.;														//cm
    TargetOffsetZ          = -92;														//cm

    TPCLx                  = 19.7;													//cm
    TPCLy                  = 31.4;													//cm
    TPCLz                  = 47.8;													//cm
      ActiveRegionx 				 = 8.8 		 												//cm
      ActiveRegiony 				 = 30.4														//cm
      ActiveRegionz 				 = 25.6
      FrameThickness	 			 = 1 														 	//cm
       */

    // World definition

    WorldSizeX = 2 * (2 * TPCLx);
    WorldSizeY = 2 * (2 * TPCLy);
    WorldSizeZ = 2 * (2 * TPCLz);

    // GTPC main area->World definition
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();

    TGeoShape* tpc_box = new TGeoBBox("GTPC_Box", WorldSizeX, WorldSizeY, WorldSizeZ);

    TGeoVolume* pWorld = new TGeoVolume("GTPC_box", tpc_box, pvacuum);

    ConstructTPC(pWorld);

    // Globle definition of TPC position in cm TODO->check the position in the simulation, this values comes from my old
    // simulation in G4
    TVector3 gTrans(10.2, 0, 284); // offset before rotation
    fGlobalTrans->SetTranslation(gTrans.X(), gTrans.Y(), gTrans.Z());
    TGeoCombiTrans* pGlobal = new TGeoCombiTrans(*fGlobalTrans, *fGlobalRot);
    top->AddNode(pWorld, 0, pGlobal);

    //____________________________________
    gGeoMan->CloseGeometry();
    gGeoMan->CheckOverlaps(0.001);
    gGeoMan->PrintOverlaps();
    gGeoMan->Test();

    cout << "vis option " << gGeoMan->GetVisOption() << " =====  vis level " << gGeoMan->GetVisLevel() << endl;

    TFile* geoFile = new TFile(geoFileName, "RECREATE");
    top->Draw("ogl");
    top->Write();
    geoFile->Close();
}
//________________________________________________________________
void ConstructTPC(TGeoVolume* pWorld)
{
    // Material-----------------------------------------------------------------
    TGeoMedium* FrameMaterial = gGeoManager->GetMedium("aluminium");
    TGeoMedium* GasMaterial = gGeoManager->GetMedium("mix");
    TGeoMedium* WindowMaterial = gGeoManager->GetMedium("mylar");
    TGeoMedium* TargetMaterial = gGeoManager->GetMedium("carbon");

    // Sensitive volume-------------------------------------------------------------------
    TGeoShape* solidTarget = 0;
    TGeoVolume* logicTarget = 0;
    TGeoShape* solidFrame = 0;
    TGeoVolume* logicFrame = 0;
    TGeoShape* solidGas = 0;
    TGeoVolume* logicGas = 0;
    TGeoShape* solidActiveRegion = 0;
    TGeoVolume* logicActiveRegion = 0;
    TGeoShape* solidFWindow = 0;
    TGeoVolume* logicFWindow = 0;
    TGeoShape* solidBWindow = 0;
    TGeoVolume* logicBWindow = 0;

    // C12 target
    solidTarget = new TGeoTubeSeg("Target", 0., TargetRadius, TargetLength, 0, 360.);
    logicTarget = new TGeoVolume("Target", solidTarget, TargetMaterial);

    pWorld->AddNode(logicTarget, 0, new TGeoCombiTrans(TargetOffsetX, TargetOffsetY, TargetOffsetZ, zeroRot));

    // Aluminium frame-------------------------------------

    TGeoShape* frame_box = new TGeoBBox("frame_box", TPCLx, TPCLy, TPCLz);

    TGeoShape* frame_window = new TGeoBBox("frame_window", 0.5 * FrameThickness, Windowy, Windowz);

    TGeoTranslation* tc1 = new TGeoTranslation("tc1", TPCLx - 0.5 * FrameThickness, 0, 0);
    tc1->RegisterYourself();

    TGeoTranslation* tc2 = new TGeoTranslation("tc2", -(TPCLx - 0.5 * FrameThickness), 0, 0);
    tc2->RegisterYourself();

    solidFrame = new TGeoCompositeShape("Frame", "frame_box -frame_window:tc1 -frame_window:tc2");
    // For the windows is necessary to make the holes!!!
    logicFrame = new TGeoVolume("Frame", solidFrame, FrameMaterial);
    pWorld->AddNode(logicFrame, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));

    // Gas region-----------------------------------------

    solidGas =
        new TGeoBBox("gas_box", TPCLx - 1 * FrameThickness, TPCLy - 1 * FrameThickness, TPCLz - 1 * FrameThickness);

    logicGas = new TGeoVolume("gas_box", solidGas, GasMaterial);
    pWorld->AddNode(logicGas, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));

    // Active region---------------------------------------

    solidActiveRegion = new TGeoBBox("Active_region", ActiveRegionx, ActiveRegiony, ActiveRegionz);

    TGeoTranslation* tc3 = new TGeoTranslation("tc3", 0, 0, 1.5);
    tc3->RegisterYourself();

    logicActiveRegion = new TGeoVolume("Active_region", solidActiveRegion, GasMaterial);
    pWorld->AddNode(logicActiveRegion, 0, new TGeoCombiTrans(*tc3, *zeroRot));

    // Mylar windows---------------------------------------

    solidFWindow = new TGeoBBox("front_window", Windowx, Windowy, Windowz);
    logicFWindow = new TGeoVolume("front_window", solidFWindow, WindowMaterial);
    pWorld->AddNode(logicFWindow, 0, new TGeoCombiTrans(-(TPCLx - FrameThickness + 0.0025), 0, 0, zeroRot));

    solidBWindow = new TGeoBBox("back_window", Windowx, Windowy, Windowz);
    logicBWindow = new TGeoVolume("back_window", solidBWindow, WindowMaterial);
    pWorld->AddNode(logicBWindow, 0, new TGeoCombiTrans(TPCLx - FrameThickness + 0.0025, 0, 0, zeroRot));

    logicTarget->SetLineColor(kAzure);
    logicFrame->SetLineColor(kBlue);
    logicGas->SetLineColor(kRed);
    logicActiveRegion->SetLineColor(kMagenta);
    logicFWindow->SetLineColor(kYellow);
    logicBWindow->SetLineColor(kOrange);
}
