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
TGeoManager* gGeoMan = NULL;

//________________________Target parameters
Double_t TargetLength;
Double_t TargetRadius;

Double_t WorldSizeX;
Double_t WorldSizeY;
Double_t WorldSizeZ;

void ConstructTPC(TGeoVolume* pWorld);

void create_tpc_geo(const char* geoTag = "Target")
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
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/Target_FileSetup.par";
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
    TString geoFileName = geoPath + "/glad-tpc/geometry/passive/";
    geoFileName = geoFileName + geoTag + ".geo.root";
    // --------------------------------------------------------------------------

    // -----------------   Get and create the required media    -----------------
    FairGeoMedia* geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

    // geoMedia->print();
    //
    FairGeoMedium* mvacuum = geoMedia->getMedium("vacuum");
    if (!mvacuum)
        Fatal("Main", "FairMedium vacuum not found");
    geoBuild->createMedium(mvacuum);
    TGeoMedium* pvacuum = gGeoMan->GetMedium("vacuum");
    if (!pvacuum)
        Fatal("Main", "Medium vacuum not found");

    FairGeoMedium* mair = geoMedia->getMedium("air");
    if (!mair)
        Fatal("Main", "FairMedium air not found");
    geoBuild->createMedium(mair);
    TGeoMedium* pair = gGeoMan->GetMedium("air");
    if (!pair)
        Fatal("Main", "Medium Air not found");

    FairGeoMedium* mLH2 = geoMedia->getMedium("LH2");
    if (!mLH2)
        Fatal("Main", "FairMedium LH2 not found");
    geoBuild->createMedium(mLH2);
    TGeoMedium* pLH2 = gGeoMan->GetMedium("LH2");
    if (!pLH2)
        Fatal("Main", "Medium LH2 not found");

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

    // --------------------------------------------------------------------------
    cout << "[INFO]: Bulding the target!!!!!\n" << endl;

    TargetLength = geoPar->GetTargetLength() / 2.; // cm
    TargetRadius = geoPar->GetTargetRadius();      // cm

    WorldSizeX = 2 * TargetLength;
    WorldSizeY = 2 * TargetLength;
    WorldSizeZ = 2 * TargetLength;

    // target world
    TGeoVolume* pAWorld = gGeoManager->GetTopVolume();

    TGeoShape* target_box = new TGeoBBox("TARGET_Box", WorldSizeX, WorldSizeY, WorldSizeZ);
    TGeoVolume* pWorld = new TGeoVolume("TARGET_Box", target_box, pvacuum);

    //__________________________
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
}
//________________________________________________________________
void ConstructTPC(TGeoVolume* pWorld)
{

    TGeoMedium* TargetMaterial = gGeoManager->GetMedium("carbon");

    // C12 target
    TGeoShape* solidTarget = 0;
    TGeoVolume* logicTarget = 0;

    solidTarget = new TGeoTubeSeg("Target", 0., TargetRadius, TargetLength, 0, 360.);
    logicTarget = new TGeoVolume("Target", solidTarget, TargetMaterial);

    pWorld->AddNode(logicTarget, 0, zeroRotTrans);

    logicTarget->SetLineColor(kAzure);
}
