#include "TFile.h"
#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoTrd2.h"
#include "TMath.h"
#include <iomanip>
#include <iostream>

TGeoRotation* zeroRot = new TGeoRotation;
TGeoTranslation* zeroTrans = new TGeoTranslation;
TGeoCombiTrans* zeroRotTrans = new TGeoCombiTrans(*zeroTrans, *zeroRot);

// Create a null translation to be used in the world
TGeoTranslation* fGlobalTrans = new TGeoTranslation();

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
Double_t GLAD_angle = 14.;
void ConstructTPC(TGeoVolume* pWorld);

void create_tpc_geo(string geoTag = "FullBeamIn")
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

    // -------   Parameters file name (input)   ----------------------------------
    TString GTPCGeoParamsFile;
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAFullBeamIn_FileSetup.par";
    GTPCGeoParamsFile.ReplaceAll("//", "/");

    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    R3BGTPCGeoPar* geoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!geoPar)
    {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }

    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    parIo1->open(GTPCGeoParamsFile, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->initContainers(0);

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
    // this is the P10 gas mixture
    FairGeoMedium* mp10 = geoMedia->getMedium("P10");
    if (!mp10)
        Fatal("Main", "FairMedium P10 not found");
    geoBuild->createMedium(mp10);
    TGeoMedium* pp10 = gGeoMan->GetMedium("P10");
    if (!pp10)
        Fatal("Main", "Medium P10 not found");
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

    //_________________________________

    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
    gGeoMan->SetName("GTPCgeom");
    TGeoVolume* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);

    top->SetVisContainers(kTRUE);
    // --------------------------------------------------------------------------

    cout << "\n \033[1;31m Warning\033[0m: the detector you are building is " << geoTag << "!!!!!\n" << endl;

    TPCLx = geoPar->GetTPCLx() / 2.;                   // cm
    TPCLy = geoPar->GetTPCLy() / 2.;                   // cm
    TPCLz = geoPar->GetTPCLz() / 2.;                   // cm
    FrameThickness = geoPar->GetFrameThickness() / 2.; // cm

    ActiveRegionx = geoPar->GetActiveRegionx() / 2.; // cm
    ActiveRegiony = geoPar->GetActiveRegiony() / 2.; // cm
    ActiveRegionz = geoPar->GetActiveRegionz() / 2.; // cm

    Windowx = geoPar->GetWindowx() / 2.; // cm
    Windowy = geoPar->GetWindowy() / 2.; // cm
    Windowz = geoPar->GetWindowz() / 2.; // cm

    /*
      TPCLx 							 	 =22.5
      //cm TPCLy 							   =22.5.
      //cm TPCLz  							   =120.
      //cm

      ActiveRegionx					 =4.
      //cm ActiveRegiony					 =30.
      //cm ActiveRegionz					 =90.
      //cm FrameThickness	 			 = 1
      //cm
      */

    // World definition

    WorldSizeX = 3.5 * TPCLx;
    WorldSizeY = 3.5 * TPCLy;
    WorldSizeZ = 2 * TPCLz;

    // GTPC main area->World definition
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();

    TGeoShape* tpc_box = new TGeoBBox("GTPC_Box", WorldSizeX, WorldSizeY, WorldSizeZ);

    TGeoVolume* pWorld = new TGeoVolume("GTPC_box", tpc_box, pvacuum);
    pWorld->SetInvisible();
    ConstructTPC(pWorld);

    //_____________________________Globle definition of TPC position in cm

    TGeoRotation* rot_chamber = new TGeoRotation();
    rot_chamber->RotateY(-GLAD_angle);
    TGeoCombiTrans* t0 = new TGeoCombiTrans(-21.5, 0., 225., rot_chamber);
    top->AddNode(pWorld, 0, t0);

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
    TGeoMedium* GasMaterial = gGeoManager->GetMedium("P10");
    TGeoMedium* WindowMaterial = gGeoManager->GetMedium("mylar");

    // Sensitive
    // volumes-------------------------------------------------------------------
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

    // Aluminium frame-----------------------------------------
    TGeoShape* frame_box = new TGeoTrd2("frame_box", 1.15 * TPCLx, 3.3 * TPCLx, 1.5 * TPCLy, 2.5 * TPCLy, TPCLz);

    TGeoShape* frame_window1 = new TGeoBBox("frame_window1", 2 * Windowx, Windowy, FrameThickness);
    TGeoShape* frame_window2 = new TGeoBBox("frame_window2", 6 * Windowx, 9 * Windowy, FrameThickness);

    // Windows translation
    TGeoTranslation* tc1 = new TGeoTranslation("tc1", 0, 0, -(TPCLz - FrameThickness));
    tc1->RegisterYourself();
    TGeoTranslation* tc2 = new TGeoTranslation("tc2", 0, 0, (TPCLz - FrameThickness));
    tc2->RegisterYourself();

    solidFrame = new TGeoCompositeShape("Frame", "frame_box -frame_window1:tc1 -frame_window2:tc2");
    logicFrame = new TGeoVolume("Frame", solidFrame, FrameMaterial);

    // Placing logicframe in the world
    pWorld->AddNode(logicFrame, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));

    // Gas region-----------------------------------------
    solidGas = new TGeoTrd2("gas",
                            1.15 * TPCLx - 2 * FrameThickness,
                            3.3 * TPCLx - 2 * FrameThickness,
                            1.5 * TPCLy - 2 * FrameThickness,
                            2.5 * TPCLy - 2 * FrameThickness,
                            TPCLz - 2 * FrameThickness);

    logicGas = new TGeoVolume("gas", solidGas, GasMaterial);

    // Placing GAS in the Frame
    logicFrame->AddNode(logicGas, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));

    // Mylar windows---------------------------------------
    solidFWindow = new TGeoBBox("front_window", 2 * Windowx, Windowy, Windowz);
    logicFWindow = new TGeoVolume("front_window", solidFWindow, WindowMaterial);

    solidBWindow = new TGeoBBox("back_window", 6 * Windowx, 9 * Windowy, Windowz);
    logicBWindow = new TGeoVolume("back_window", solidBWindow, WindowMaterial);

    // Placing logicWindows in the Frame
    logicFrame->AddNode(logicFWindow, 0, new TGeoCombiTrans(*tc1, *zeroRot));
    logicFrame->AddNode(logicBWindow, 0, new TGeoCombiTrans(*tc2, *zeroRot));

    TGeoRotation rot_Aregion; // Active region rotation
    rot_Aregion.RotateY(180);
    TGeoRotation rot_Aregion1; // Active region rotation, only v2
    rot_Aregion1.RotateY(180);

    Double_t volume[] = { -3 * ActiveRegionx, ActiveRegiony,  ActiveRegionx,      ActiveRegiony,
                          ActiveRegionx,      -ActiveRegiony, -3 * ActiveRegionx, -ActiveRegiony,
                          -ActiveRegionx,     ActiveRegiony,  ActiveRegionx,      ActiveRegiony,
                          ActiveRegionx,      -ActiveRegiony, -ActiveRegionx,     -ActiveRegiony };
    solidActiveRegion = new TGeoArb8("Active_region", ActiveRegionz, volume);
    rot_Aregion.RotateY(GLAD_angle);

    // Logical volume active region
    logicActiveRegion = new TGeoVolume("Active_region", solidActiveRegion, GasMaterial);

    // Placing logicActiveRegion inside GAS
    logicGas->AddNode(logicActiveRegion,
                      0,
                      new TGeoCombiTrans(29.8,
                                         0.,
                                         0,
                                         new TGeoRotation(rot_Aregion))); // 21.5+8.64

    // Some colours to better distinguish the logic volumes
    logicFrame->SetLineColor(kBlue);
    logicGas->SetLineColor(kRed);
    logicFWindow->SetLineColor(kYellow);
    logicBWindow->SetLineColor(kOrange);
    logicActiveRegion->SetLineColor(kMagenta);
}
