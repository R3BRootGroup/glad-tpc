#include "R3BGTPCEventDrawTask.h"
#include "R3BGTPCEventManager.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"
#include "TColor.h"
#include "TEveBoxSet.h"
#include "TEveGeoShape.h"
#include "TEveLine.h"
#include "TEveManager.h"
#include "TEvePointSet.h"
#include "TEveTrans.h"
#include "TF1.h"
#include "TGeoSphere.h"
#include "TH1.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TH3.h"
#include "TPaletteAxis.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TVector3.h"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"
#define cGREEN "\033[1;32m"
#define cBLUE "\033[1;34m"

ClassImp(R3BGTPCEventDrawTask);

R3BGTPCEventDrawTask::R3BGTPCEventDrawTask()
    : fCvsPadPlane(0)
    , fPadPlane(0)
    , fMap(0)
    , fHitSet(0)
    , fCvsPadWave(0)
    , fPadWave(0)
    , fHitCA(0)
    , fTrackCA(0)
{
}

R3BGTPCEventDrawTask::~R3BGTPCEventDrawTask() {}

InitStatus R3BGTPCEventDrawTask::Init()
{

    std::cout << " =====  R3BGTPCEventDrawTask::Init =====" << std::endl;

    gROOT->Reset();
    FairRootManager* ioMan = FairRootManager::Instance();
    fEventManager = R3BGTPCEventManager::Instance();

    fMap = new R3BGTPCMap();

    // Data
    fHitCA = (TClonesArray*)ioMan->GetObject("GTPCHitData");
    if (fHitCA)
        LOG(info) << cGREEN << "Hit Array Found in branch GTPCHitData." << cNORMAL << std::endl;

    fTrackCA = (TClonesArray*)ioMan->GetObject("GTPCTrackData");
    if (fTrackCA)
        LOG(info) << cGREEN << "Track Array Found in branch GTPCTrackData." << cNORMAL << std::endl;

    // Canvas
    fCvsPadPlane = fEventManager->GetCvsPadPlane();

    // Pad Plane
    DrawPadPlane();

    std::cout << " R3BGTPCEventDrawTask::Init : Initialization complete! "
              << "\n";

    return kSUCCESS;
}

void R3BGTPCEventDrawTask::Exec(Option_t* option)
{
    Reset();

    if (fHitCA)
    {
        DrawHitPoints();
    }

    if (fTrackCA)
    {
        DrawTracks();
    }

    UpdateCvsPadPlane();

    gEve->Redraw3D(kTRUE);
}

void R3BGTPCEventDrawTask::Reset()
{

    if (fHitSet)
    {
        fHitSet->Reset();
        gEve->RemoveElement(fHitSet, fEventManager);
    }

    if (fTrackCA)
    {
        for (auto elem : fTrackHitSet)
        {
            gEve->RemoveElement(elem, fEventManager);
        }

        fTrackHitSet.clear();

        for (auto elem : fHitClusterSet)
            gEve->RemoveElement(elem, fEventManager);

        fHitClusterSet.clear();
    }

    if (fPadPlane != NULL)
        fPadPlane->Reset(0);
}

void R3BGTPCEventDrawTask::DrawTracks()
{
    auto nTracks = fTrackCA->GetEntries();
    std::cout << " Found " << nTracks << " tracks."
              << "\n";

    R3BGTPCTrackData** trackData;
    trackData = new R3BGTPCTrackData*[nTracks];

    for (auto i = 0; i < nTracks; ++i)
    {
        trackData[i] = (R3BGTPCTrackData*)(fTrackCA->At(i));
        Color_t trackColor = GetTrackColor(i) + 1;
        fTrackHitSet.push_back(new TEvePointSet(Form("TrackHit_%d", i), 0, TEvePointSelectorConsumer::kTVT_XYZ));
        fTrackHitSet[i]->SetMarkerColor(trackColor);
        fTrackHitSet[i]->SetMarkerSize(1);
        fTrackHitSet[i]->SetMarkerStyle(kFullDotMedium);

        fHitClusterSet.push_back(new TEveBoxSet(Form("HitCluster_%d", i)));
        fHitClusterSet[i]->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

        std::vector<R3BGTPCHitData> trackHits = trackData[i]->GetHitArray();
        std::vector<R3BGTPCHitClusterData>* trackClusterHits = trackData[i]->GetHitClusterArray();

        for (auto& trackHit : trackHits)
        {
            Double_t x = trackHit.GetX();
            Double_t y = trackHit.GetY();
            Double_t z = trackHit.GetZ();
            Double_t E = trackHit.GetEnergy();

            fTrackHitSet[i]->SetNextPoint(x, y, z);
        }

        std::cout << trackClusterHits->size() << "\n";

        if (trackClusterHits->size() > 0)
        {

            for (auto trackHitCluster : *trackClusterHits)
            {
                Double_t xC = trackHitCluster.GetX();
                Double_t yC = trackHitCluster.GetY();
                Double_t zC = trackHitCluster.GetZ();
                Double_t EC = trackHitCluster.GetEnergy();

                double boxSize = 0.6;

                fHitClusterSet[i]->AddBox(
                    xC - boxSize / 2.0, yC - boxSize / 2.0, zC - boxSize / 2.0, boxSize, boxSize, boxSize);
            }

            fHitClusterSet[i]->UseSingleColor();
            fHitClusterSet[i]->SetMainColor(trackColor);
            fHitClusterSet[i]->SetMainTransparency(50);

            fHitClusterSet[i]->RefitPlex();
            TEveTrans& tHitClusterBoxPos = fHitClusterSet[i]->RefMainTrans();
            tHitClusterBoxPos.SetPos(0.0, 0.0, 0.0);
        }
    }

    for (auto& elem : fTrackHitSet)
        gEve->AddElement(elem);
    for (auto& elem : fHitClusterSet)
        gEve->AddElement(elem);
}

void R3BGTPCEventDrawTask::DrawHitPoints()
{

    // Drawing Hit Pattern
    fHitBoxSet = new TEveBoxSet("hitBox");
    fHitBoxSet->Reset(TEveBoxSet::kBT_AABox, kTRUE, 64);

    auto nHits = fHitCA->GetEntries();
    std::cout << " Found " << nHits << " hits."
              << "\n";
    fHitSet = new TEvePointSet("Hit", nHits, TEvePointSelectorConsumer::kTVT_XYZ);
    fHitSet->SetOwnIds(kTRUE);
    fHitSet->SetMarkerColor(kRed);
    fHitSet->SetMarkerSize(1);
    fHitSet->SetMarkerStyle(kFullDotMedium);

    R3BGTPCHitData** hitData;
    hitData = new R3BGTPCHitData*[nHits];

    for (Int_t i = 0; i < nHits; i++)
    {
        hitData[i] = (R3BGTPCHitData*)(fHitCA->At(i));
        Double_t x = hitData[i]->GetX();
        Double_t y = hitData[i]->GetY();
        Double_t z = hitData[i]->GetZ();
        Double_t E = hitData[i]->GetEnergy();

        // std::cout<<x<<" "<<y<<" "<<z<<" "<<"\n";

        fHitSet->SetNextPoint(x / 10.0, y / 10.0, z / 10.0); // cm
        fHitSet->SetPointId(new TNamed(Form("Hit %d", i), ""));
        // NB: X and Z are swapped in the pad plane
        //    PadPlane frame for visualization
        double ZOffset = 272.7;
        double XOffset = 5.8;
        fPadPlane->Fill((z - ZOffset) * 10.0, (x - XOffset) * 10.0, E);
    }

    gEve->AddElement(fHitSet);
}

void R3BGTPCEventDrawTask::DrawPadPlane()
{

    if (fPadPlane)
    {
        fPadPlane->Reset(0);
        return;
    }

    fMap->GeneratePadPlane();
    fPadPlane = fMap->GetPadPlane();
    fCvsPadPlane->cd();
    // fPadPlane -> Draw("COLZ L0"); //0  == bin lines adre not drawn
    fPadPlane->Draw("COL L");
    fPadPlane->SetMinimum(1.0);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(103);
    gPad->Update();
}

void R3BGTPCEventDrawTask::UpdateCvsPadPlane()
{
    fCvsPadPlane->Modified();
    fCvsPadPlane->Update();
}

EColor R3BGTPCEventDrawTask::GetTrackColor(int i)
{
    std::vector<EColor> colors = { kAzure, kOrange, kViolet, kTeal, kMagenta, kBlue, kViolet, kYellow, kCyan, kAzure };
    if (i < 10)
    {
        return colors.at(i);
    }
    else
        return kAzure;
}
