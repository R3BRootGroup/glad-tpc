#include "R3BGTPCEventManager.h"
#include "FairEventManagerEditor.h"

#include "TEveBrowser.h"
#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TEveWindow.h"

#include "TRootEmbeddedCanvas.h"

#include "TGButton.h"
#include "TGCanvas.h"
#include "TGLViewer.h"
#include "TGLabel.h"
#include "TGTab.h"
#include "TGWidget.h"
#include "TGWindow.h"
#include "TGeoManager.h"
#include "TVirtualX.h"

#include "TCanvas.h"
#include "TEveGedEditor.h"
#include "TGLEmbeddedViewer.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TMultiGraph.h"
#include "TObject.h"
#include "TPolyLine.h"
#include "TROOT.h"
#include "TStyle.h"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"
#define cGREEN "\033[1;32m"
#define cBLUE "\033[1;34m"
#define cWHITERED "\033[37;41m"

#include <iostream>

class TGeoNode;

ClassImp(R3BGTPCEventManager);

R3BGTPCEventManager* R3BGTPCEventManager::fInstance = 0;
R3BGTPCEventManager* R3BGTPCEventManager::Instance() { return fInstance; }

R3BGTPCEventManager::R3BGTPCEventManager()
    : TEveEventManager("R3BGTPCEventManager", "")
    , fRootManager(FairRootManager::Instance())
    , fRunAna(FairRunAna::Instance())
    , fEntry(0)
    , fEvent(0)
    , fCurrentEvent(0)
    , fCvsPadPlane(0)
    , fPadWave(0)

{
    fInstance = this;
}

R3BGTPCEventManager::~R3BGTPCEventManager() {}

void R3BGTPCEventManager::Init(Int_t option, Int_t level, Int_t nNodes)
{

    gStyle->SetOptTitle(0);
    gStyle->SetPalette(55);
    TEveManager::Create();

    Int_t dummy;
    UInt_t width, height;
    UInt_t widthMax = 1400, heightMax = 650;
    Double_t ratio = (Double_t)widthMax / heightMax;
    gVirtualX->GetWindowSize(gClient->GetRoot()->GetId(), dummy, dummy, width, height);
    // Assume that width of screen is always larger than the height of screen
    if (width > widthMax)
    {
        width = widthMax;
        height = heightMax;
    }
    else
        height = (Int_t)(width / ratio);
    // gEve->GetMainWindow()->Resize(width,height);

    /**************************************************************************/

    TEveWindowSlot* slot = 0;
    TEveWindowPack* pack = 0;

    // 3D
    slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
    pack = slot->MakePack();
    pack->SetElementName("GTPC 3D/Pad plane views");
    pack->SetHorizontal();
    // pack->SetVertical();
    pack->SetShowTitleBar(kFALSE);

    pack->NewSlot()->MakeCurrent();
    // if(kDraw3DGeo){
    TEveViewer* view3D = gEve->SpawnNewViewer("3D View", "");
    view3D->AddScene(gEve->GetGlobalScene());
    view3D->AddScene(gEve->GetEventScene());
    // }

    slot = pack->NewSlot();
    TEveWindowPack* pack2 = slot->MakePack();
    pack2->SetShowTitleBar(kFALSE);
    pack2->SetVertical();
    slot = pack2->NewSlot();
    slot->StartEmbedding();
    fPadWave = new TCanvas("R3BGTPCPad Canvas");
    fPadWave->ToggleEditor();
    slot->StopEmbedding();

    // Pad Plane
    slot = pack2->NewSlotWithWeight(1.5);
    TRootEmbeddedCanvas* ecvs = new TRootEmbeddedCanvas();
    TEveWindowFrame* frame = slot->MakeFrame(ecvs);
    frame->SetElementName("R3BGTPC Pad Plane");
    pack->GetEveFrame()->SetShowTitleBar(kFALSE);
    fCvsPadPlane = ecvs->GetCanvas();

    fRunAna->Init();

    if (gGeoManager)
    {

        std::cout << " GeoManager found "
                  << "\n";

        TGeoNode* geoNode = gGeoManager->GetTopNode();
        TEveGeoTopNode* topNode = new TEveGeoTopNode(gGeoManager, geoNode, option, level, nNodes);
        gEve->AddGlobalElement(topNode);

        Int_t transparency = 80;

        TObjArray* listVolume = gGeoManager->GetListOfVolumes();
        Int_t nVolumes = listVolume->GetEntries();

        for (Int_t i = 0; i < nVolumes; i++)
            ((TGeoVolume*)listVolume->At(i))->SetTransparency(transparency);

        gEve->FullRedraw3D(kTRUE);
        fEvent = gEve->AddEvent(this);
    }

    /**************************************************************************/

    gEve->GetBrowser()->GetTabRight()->SetTab(1);
    make_gui();

    gEve->Redraw3D(kTRUE, kTRUE);

    TGLViewer* dfViewer = gEve->GetDefaultGLViewer(); // Is this doing anything?
    dfViewer->CurrentCamera().RotateRad(-.7, 0.5);
    dfViewer->DoDraw();
}

void R3BGTPCEventManager::AddTask(FairTask* task) { fRunAna->AddTask(task); }

void R3BGTPCEventManager::SelectEvent()
{
    GotoEvent(fCurrentEvent->GetIntNumber());
    // cout<<fCurrentEvent->GetIntNumber()<<endl;
}

void R3BGTPCEventManager::GotoEvent(Int_t event)
{

    fEntry = event;
    std::cout << cWHITERED << " Event number : " << fEntry << cNORMAL << std::endl;
    fRunAna->Run((Long64_t)event);
}

void R3BGTPCEventManager::NextEvent()
{
    fEntry += 1;
    std::cout << " Event number : " << fEntry << std::endl;
    fRunAna->Run((Long64_t)fEntry);
}

void R3BGTPCEventManager::PrevEvent()
{
    fEntry -= 1;
    std::cout << " Event number : " << fEntry << std::endl;
    fRunAna->Run((Long64_t)fEntry);
}

void R3BGTPCEventManager::RunEvent() { fRunAna->Run((Long64_t)fEntry); }

void R3BGTPCEventManager::make_gui()
{

    TChain* chain = FairRootManager::Instance()->GetInChain();
    Int_t Entries = chain->GetEntriesFast();

    TEveBrowser* browser = gEve->GetBrowser();
    browser->StartEmbedding(TRootBrowser::kLeft);

    TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
    frmMain->SetWindowName("XX GUI");
    frmMain->SetCleanup(kDeepCleanup);

    TGVerticalFrame* hf = new TGVerticalFrame(frmMain);
    {
    }

    TGHorizontalFrame* hf_2 = new TGHorizontalFrame(frmMain);
    {
    }

    frmMain->AddFrame(hf);
    frmMain->AddFrame(hf_2);

    TString Infile = "Input file : ";
    TFile* file = FairRootManager::Instance()->GetInChain()->GetFile();
    Infile += file->GetName();
    TGLabel* TFName = new TGLabel(frmMain, Infile.Data());
    frmMain->AddFrame(TFName);

    UInt_t RunId = FairRunAna::Instance()->getRunId();
    TString run = "Run Id : ";
    run += RunId;
    TGLabel* TRunId = new TGLabel(frmMain, run.Data());
    frmMain->AddFrame(TRunId);

    TString nevent = "No of events : ";
    nevent += Entries;
    TGLabel* TEvent = new TGLabel(frmMain, nevent.Data());
    frmMain->AddFrame(TEvent);

    TGHorizontalFrame* f = new TGHorizontalFrame(frmMain);
    TGLabel* l = new TGLabel(f, "Current Event:");
    f->AddFrame(l, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));

    fCurrentEvent = new TGNumberEntry(f,
                                      0.,
                                      6,
                                      -1,
                                      TGNumberFormat::kNESInteger,
                                      TGNumberFormat::kNEANonNegative,
                                      TGNumberFormat::kNELLimitMinMax,
                                      0,
                                      Entries);
    f->AddFrame(fCurrentEvent, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));
    fCurrentEvent->Connect("ValueSet(Long_t)", "R3BGTPCEventManager", fInstance, "SelectEvent()");
    frmMain->AddFrame(f);

    /*TGHorizontalFrame *fThres = new TGHorizontalFrame(frmMain);
    TGLabel *lThres = new TGLabel(fThres, "3D threshold:");
    fThres->AddFrame(lThres, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2,
    1, 1)); f3DThresDisplay = new TGNumberEntry(fThres, 0., 6, -1,
    TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                        TGNumberFormat::kNELLimitMinMax, 0, Entries);
    fThres->AddFrame(f3DThresDisplay, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));
    f3DThresDisplay->Connect("ValueSet(Long_t)", "R3BGTPCEventManager", fInstance,
    "Select3DThres()"); frmMain->AddFrame(fThres);*/

    frmMain->MapSubwindows();
    frmMain->Resize();
    frmMain->MapWindow();

    browser->StopEmbedding();
    browser->SetTabTitle("GTPC Event Control", 0);
}

// void R3BGTPCEventManager::Select3DThres()
//{

// k3DThreshold = f3DThresDisplay->GetIntNumber();
//}
