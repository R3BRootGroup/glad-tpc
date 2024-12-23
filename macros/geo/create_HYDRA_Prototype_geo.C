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

    // -------   Parameters file name (input)   ----------------------------------
    TString GTPCGeoParamsFile;
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par";
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
    TString geoFileManName = geoFileName + geoTag.c_str() + ".geoMan.root";
    geoFileName = geoFileName + geoTag.c_str() + ".geo.root";

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

    ActiveRegionx = geoPar->GetActiveRegionx() / 2.; // cm
    ActiveRegiony = geoPar->GetActiveRegiony() / 2.; // cm
    ActiveRegionz = geoPar->GetActiveRegionz() / 2.; // cm

    TPCLx = geoPar->GetTPCLx() / 2.;                   // cm
    TPCLy = geoPar->GetTPCLy() / 2.;                   // cm
    TPCLz = geoPar->GetTPCLz() / 2.;                   // cm
    FrameThickness = geoPar->GetFrameThickness() / 2.; // cm

    Windowx = geoPar->GetWindowx() / 2.; // cm
    Windowy = geoPar->GetWindowy() / 2.; // cm
    Windowz = geoPar->GetWindowz() / 2.; // cm

    // World definition

    WorldSizeX = 3 * TPCLx;
    WorldSizeY = 3 * TPCLy;
    WorldSizeZ = 3 * TPCLz;

    // GTPC main area->World definition
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();
    TGeoShape* tpc_box = new TGeoBBox("GTPC_Box", WorldSizeX, WorldSizeY, WorldSizeZ);

    TGeoVolume* pWorld = new TGeoVolume("GTPC_box", tpc_box, pvacuum);
    pWorld->SetInvisible();
    ConstructTPC(pWorld);

    top->AddNode(pWorld, 0, zeroRotTrans);

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

    TFile* geoFileMan = new TFile(geoFileManName, "RECREATE");
    gGeoMan->Write();
    geoFileMan->Close();
}
//________________________________________________________________
void ConstructTPC(TGeoVolume* pWorld)
{
    // Material-----------------------------------------------------------------
    TGeoMedium* FrameMaterial = gGeoManager->GetMedium("aluminium");
    TGeoMedium* GasMaterial = gGeoManager->GetMedium("P10");
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
    TGeoShape* frame_window = new TGeoBBox("frame_window", 2 * FrameThickness, Windowy, Windowz);

    TGeoTranslation* tc1 = new TGeoTranslation("tc1", TPCLx, 0, 0);
    tc1->RegisterYourself();
    TGeoTranslation* tc2 = new TGeoTranslation("tc2", -(TPCLx), 0, 0);
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

    TGeoTranslation* tc3 = new TGeoTranslation("tc3",
                                               0,
                                               0,
                                               2); // Active region not centered inside the chamber, 2 cm shift
    tc3->RegisterYourself();

    // Mylar windows------------------------------------------------------------

    solidFWindow = new TGeoBBox("front_window", Windowx, Windowy, Windowz);
    logicFWindow = new TGeoVolume("front_window", solidFWindow, WindowMaterial);

    solidBWindow = new TGeoBBox("back_window", Windowx, Windowy, Windowz);
    logicBWindow = new TGeoVolume("back_window", solidBWindow, WindowMaterial);

    // Positioning the volumes in the world
    // frame
    pWorld->AddNode(logicFrame, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));
    // Positioning the volumes in the frame
    // front_window
    logicFrame->AddNode(logicFWindow, 0, new TGeoCombiTrans(-(TPCLx - 2 * FrameThickness + 0.0025), 0, 0, zeroRot));
    // back_window
    logicFrame->AddNode(logicBWindow, 0, new TGeoCombiTrans(TPCLx - 2 * FrameThickness + 0.0025, 0, 0, zeroRot));
    // GAS
    logicFrame->AddNode(logicGas, 0, new TGeoCombiTrans(0., 0., 0, zeroRot));
    // Positioning the volumes in the gas_box
    // Active_region
    logicGas->AddNode(logicActiveRegion, 0, new TGeoCombiTrans(*tc3, *zeroRot));

    logicFrame->SetLineColor(kBlue);
    logicGas->SetLineColor(kRed);
    logicActiveRegion->SetLineColor(kMagenta);
    logicFWindow->SetLineColor(kYellow);
    logicBWindow->SetLineColor(kOrange);
}
