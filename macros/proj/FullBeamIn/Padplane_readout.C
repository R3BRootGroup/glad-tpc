/*
Author: Simone Velardita
Date:		09/02/2021
Macro for the padplane readout
HOW TO USE:
bla bla
*/
#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TSystem.h>
#include <math.h>
// Loading bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void loadfunction(double& percentage)
{
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
using namespace std;

void reader(const char* inputSimFile)
{

    // SETUP
    string geoTag = "Prototype";
    cout << "\n \033[1;31m Warning\033[0m: the detector you are building is " << geoTag << "!!!!!\n" << endl;
    TString GTPCGeoParamsFile;
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";
    GTPCGeoParamsFile.ReplaceAll("//", "/");

    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    R3BGTPCGeoPar* geoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!geoPar)
    {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }
    R3BGTPCGasPar* gasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!gasPar)
    {
        cout << "No R3BGTPCGasPar can be loaded from the rtdb";
        return;
    }

    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    parIo1->open(GTPCGeoParamsFile, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->initContainers(0);

    Double_t fHalfSizeTPC_X = geoPar->GetActiveRegionx() / 2.; // X (row)
    Double_t fHalfSizeTPC_Y = geoPar->GetActiveRegiony() / 2.; // Y (time)
    Double_t fHalfSizeTPC_Z = geoPar->GetActiveRegionz() / 2.; // Z (column)
    Double_t fSizeOfVirtualPad = geoPar->GetPadSize();         // 1: pads of 1cm^2 , 10: pads of 1mm^2
    Double_t fMaxDriftTime = (round)((geoPar->GetActiveRegiony() / gasPar->GetDriftVelocity()) * pow(10, -3)); // us
    // root style
    gROOT->SetStyle("Default");
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);

    cout << "Reading input simulation file " << inputSimFile << endl;
    // HERE THE HISTOGRAMS ARE DEFINED...
    // IT IS POSSIBLE TO CHANGE THE RANGE AND THE BINNING
    UInt_t histoBins = 2 * fHalfSizeTPC_X * fSizeOfVirtualPad;
    UInt_t histoBins2 = 2 * fHalfSizeTPC_Z * fSizeOfVirtualPad;

    cout << "Values taken for the visualization" << endl
         << "bins in X: " << histoBins << endl
         << "bins in Z: " << histoBins2 << endl
         << "X size (half-length of box): " << fHalfSizeTPC_X << endl
         << "Y size (half-length of box (drift)): " << fHalfSizeTPC_Y << endl
         << "Z size (half-length of box): " << fHalfSizeTPC_Z << endl
         << "Max Drift time: " << fMaxDriftTime << endl;

    // The th2 definition
    TH2D* htrackInPads = 0;
    TH2D* hdriftTimeInPads = 0;
    TH2D* hdepth1InPads = 0;
    TH2D* hdepth2InPads = 0;
    TH2D* Hitmap = 0;

    htrackInPads = new TH2D("htrackInPads",
                            "All tracks in the XZ Pads Plane",
                            histoBins2,
                            0,
                            2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                            histoBins,
                            0,
                            2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    htrackInPads->SetYTitle("X [pad number]");
    htrackInPads->SetXTitle("Z [pad number]");

    hdriftTimeInPads = new TH2D("hdriftTimeInPads",
                                "All tracks in the XZ Pads Plane with drift time",
                                histoBins2,
                                0,
                                2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                                histoBins,
                                0,
                                2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    hdriftTimeInPads->SetYTitle("X [pad number]");
    hdriftTimeInPads->SetXTitle("Z [pad number]");

    hdepth1InPads = new TH2D("hdepth1InPads",
                             "track In the Drift-Z Pads Plane",
                             histoBins,
                             0,
                             fMaxDriftTime,
                             histoBins2,
                             0,
                             2 * fHalfSizeTPC_Z * fSizeOfVirtualPad);
    hdepth1InPads->SetYTitle("Z [pad number]");
    hdepth1InPads->SetXTitle("(drift) time [us]");

    hdepth2InPads = new TH2D("hdepth2InPads",
                             "track In the Drift-X Pads Plane",
                             histoBins,
                             0,
                             fMaxDriftTime,
                             histoBins,
                             0,
                             2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
    hdepth2InPads->SetYTitle("X [pad number]");
    hdepth2InPads->SetXTitle("(drift) time [us]");

    Hitmap = new TH2D("Hitmap",
                      "Hitmap XZ Pads Plane",
                      histoBins2,
                      0,
                      2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                      histoBins,
                      0,
                      2 * fHalfSizeTPC_X * fSizeOfVirtualPad); // in [pad number]
    Hitmap->SetYTitle("X [pad number]");
    Hitmap->SetXTitle("Z [pad number]");
    Hitmap->SetTitleOffset(2., "XYZ");

    TFile* simFile = TFile::Open(inputSimFile);
    if (simFile->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    TTree* TEvt = (TTree*)simFile->Get("evt");
    Int_t nevents = TEvt->GetEntries();

    cout << "nevents=" << nevents << endl;

    // GTPCProjPoints
    TClonesArray* gtpcProjPointCA;
    R3BGTPCProjPoint* ppoint = new R3BGTPCProjPoint;
    gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
    branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

    Int_t padsPerEvent = 0;
    Int_t nb = 0;
    Int_t beamPadsWithSignalPerEvent, productPadsWithSignalPerEvent;
    Double_t xPad, zPad, tPad;

    // all events
    for (Int_t i = 0; i < nevents; i++)
    {
        double percentage = i / (double)(nevents * 1.);
        loadfunction(percentage);
        gtpcProjPointCA->Clear();
        nb += TEvt->GetEvent(i);
        padsPerEvent = gtpcProjPointCA->GetEntries();
        if (padsPerEvent > 0)
        {
            for (Int_t h = 0; h < padsPerEvent; h++)
            {
                ppoint = (R3BGTPCProjPoint*)gtpcProjPointCA->At(h);

                if (geoTag.compare("Prototype") == 0)
                {
                    xPad = ppoint->GetVirtualPadID() % (Int_t)(100);
                    zPad = (ppoint->GetVirtualPadID() - xPad) / (100);
                }
                else
                {
                    xPad = ppoint->GetVirtualPadID() % (Int_t)(2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                    zPad = (ppoint->GetVirtualPadID() - xPad) / (2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                }
                tPad = ((TH1S*)(ppoint->GetTimeDistribution()))->GetMean();
                hdriftTimeInPads->Fill(zPad, xPad, tPad); // NOTE: THAT IS ACCUMULATED TIME!!.
                htrackInPads->Fill(zPad, xPad, ppoint->GetCharge());
                hdepth1InPads->Fill(tPad, zPad, ppoint->GetCharge());
                hdepth2InPads->Fill(tPad, xPad, ppoint->GetCharge());

                Hitmap->Fill(zPad, xPad);
            }
        }
    }
    cout << "\n" << endl;
    TCanvas* c3 = new TCanvas("c3", "Pads in pad (XZ) plane", 0, 0, 1250, 950);
    TLatex l;
    l.SetTextAlign(12);
    l.SetTextSize(0.05);

    c3->SetFillColor(0);
    c3->Divide(2, 2);
    c3->Draw();
    TVirtualPad* c3_1 = c3->cd(1);
    c3_1->SetLogz();
    htrackInPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.2 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
                2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad,
                "Color code: induced charge");

    TVirtualPad* c3_2 = c3->cd(2);
    // c3_2->SetLogz();
    hdriftTimeInPads->Draw("ZCOL");

    TLatex l2;
    l2.SetTextAlign(12);
    l2.SetTextSize(0.05);
    l2.DrawLatex(
        0.2 * fHalfSizeTPC_Z * fSizeOfVirtualPad, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: drift time");

    TVirtualPad* c3_3 = c3->cd(3);
    c3_3->SetLogz();
    hdepth1InPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3 * fMaxDriftTime, 2.04 * fHalfSizeTPC_Z * fSizeOfVirtualPad, "Color code: induced charge");

    TVirtualPad* c3_4 = c3->cd(4);
    c3_4->SetLogz();
    hdepth2InPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3 * fMaxDriftTime, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: induced charge");

    gPad->Modified();
    gPad->Update();

    TCanvas* c1 = new TCanvas("c1", "Hitmap", 0, 0, 1800, 1500);
    c1->Divide(1, 2);
    c1->cd(1);
    Hitmap->Draw("COLZ");
    c1->cd(2);
    Hitmap->Draw("lego2");
    c1->Draw();

    // OUTPUT FILE
    c3->Print("output_readout.ps(");
    c1->Print("output_readout.ps)");
    // c2->Print("output.ps");
    // c3->Print("output.ps)");
}
