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

TGeoManager* gGeoMan = NULL;

//________________________HYDRA parameters
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

void create_tpc_geo(string geoTag = "Prototype")
{ 
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
    //_________________________________

    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
    gGeoMan->SetName("GTPCgeom");
    TGeoVolume* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);

    top->SetVisContainers(kTRUE);
    gGeoManager->SetVisLevel(4);
    gGeoManager->SetVisOption(0);
    // --------------------------------------------------------------------------

    cout << "\n \033[1;31m Warning\033[0m: the detector you are building is " << geoTag << "!!!!!\n" << endl;
    R3BGTPCSetup* setup = new R3BGTPCSetup(geoTag, 1);

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
    TPCLx                  = 19.7;													//cm
    TPCLy                  = 31.4;													//cm
    TPCLz                  = 47.8;													//cm
    ActiveRegionx 				 = 8.8 		 												//cm
    ActiveRegiony 				 = 30.4														//cm
    ActiveRegionz 				 = 25.6
    FrameThickness	 			 = 1 														 	//cm
       */

    // World definition

    WorldSizeX =  2 * TPCLx;
    WorldSizeY = 	2 * TPCLy;
    WorldSizeZ =	2 * TPCLz;

    // GTPC main area->World definition
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();
    TGeoShape* tpc_box = new TGeoBBox("GTPC_Box", WorldSizeX, WorldSizeY, WorldSizeZ);

    TGeoVolume* pWorld = new TGeoVolume("GTPC_box", tpc_box, pvacuum);
		pWorld->SetInvisible();
    ConstructTPC(pWorld);

    // Globle definition of TPC position in cm 
    TVector3 gTrans(10.2, 0, 284.); // offset before rotation
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

    // Sensitive volume---------------------------------------------------------
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

    // Aluminium frame----------------------------------------------------------

    TGeoShape* frame_box = new TGeoBBox("frame_box", TPCLx, TPCLy, TPCLz);
    TGeoShape* frame_window = new TGeoBBox("frame_window", FrameThickness, Windowy, Windowz);

    TGeoTranslation* tc1 = new TGeoTranslation("tc1", TPCLx - FrameThickness, 0, 0);
    tc1->RegisterYourself();
    TGeoTranslation* tc2 = new TGeoTranslation("tc2", -(TPCLx - FrameThickness), 0, 0);
    tc2->RegisterYourself();

    solidFrame = new TGeoCompositeShape("Frame", "frame_box -frame_window:tc1 -frame_window:tc2");
    logicFrame = new TGeoVolume("Frame", solidFrame, FrameMaterial);
    // Gas region---------------------------------------------------------------

    solidGas =
        new TGeoBBox("gas_box", TPCLx - 2 * FrameThickness, TPCLy - 2 * FrameThickness, TPCLz - 2 * FrameThickness);
		logicGas = new TGeoVolume("gas_box", solidGas, GasMaterial);
    // Active region------------------------------------------------------------

    solidActiveRegion = new TGeoBBox("Active_region", ActiveRegionx, ActiveRegiony, ActiveRegionz);
    logicActiveRegion = new TGeoVolume("Active_region", solidActiveRegion, GasMaterial);
    
    TGeoTranslation* tc3 = new TGeoTranslation("tc3", 0, 0, 1.5);
    tc3->RegisterYourself();

    // Mylar windows------------------------------------------------------------

    solidFWindow = new TGeoBBox("front_window", Windowx, Windowy, Windowz);
    logicFWindow = new TGeoVolume("front_window", solidFWindow, WindowMaterial);

    solidBWindow = new TGeoBBox("back_window", Windowx, Windowy, Windowz);
    logicBWindow = new TGeoVolume("back_window", solidBWindow, WindowMaterial);
		
		//Positioning the volumes in the world
		//frame
		pWorld->AddNode(logicFrame, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));
		//Positioning the volumes in the frame		
		//front_window
    logicFrame->AddNode(logicFWindow, 0, new TGeoCombiTrans(-(TPCLx - 2 * FrameThickness + 0.0025), 0, 0, zeroRot));
    //back_window
    logicFrame->AddNode(logicBWindow, 0, new TGeoCombiTrans(TPCLx - 2 * FrameThickness + 0.0025, 0, 0, zeroRot));
    //GAS
    logicFrame->AddNode(logicGas, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));
		//Positioning the volumes in the gas_box
		//Active_region
    logicGas->AddNode(logicActiveRegion, 0, new TGeoCombiTrans(*tc3, *zeroRot));	    
		
    logicFrame->SetLineColor(kBlue);
    logicGas->SetLineColor(kRed);
    logicActiveRegion->SetLineColor(kMagenta);
    logicFWindow->SetLineColor(kYellow);
    logicBWindow->SetLineColor(kOrange);
}
