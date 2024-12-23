/*
Author: Simone Velardita
Date:		09/06/2021
TODO ->Needs to be ultimate
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
const double e_equ = 1.60217e-19; // electron charge to coulombs

void loadfunction(double& percentage)
{
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
using namespace std;

void reader(int evtnumber = 9)
{
    TStopwatch timer;
    timer.Start();
    Bool_t kAllEvents = kTRUE;
    // SETUP
    string geoTag = "Prototype";
    const char* inputSimFile = "ele_output.root";

    TString GTPCGeoParamsFile;
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";
    GTPCGeoParamsFile2 = geoPath + "/glad-tpc/params/Electronic_FileSetup.par";
    GTPCGeoParamsFile.ReplaceAll("//", "/");
    GTPCGeoParamsFile2.ReplaceAll("//", "/");

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
    R3BGTPCElecPar* elecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!elecPar)
    {
        cout << "No R3BGTPCElecPar can be loaded from the rtdb";
        return;
    }

    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    parIo1->open(GTPCGeoParamsFile, "in");
    parIo2->open(GTPCGeoParamsFile2, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->setSecondInput(parIo2);
    rtdb->initContainers(0);

    Double_t fHalfSizeTPC_X = geoPar->GetActiveRegionx() / 2.; // X (row) 		[cm]
    Double_t fHalfSizeTPC_Y = geoPar->GetActiveRegiony() / 2.; // Y (time) 	[cm]
    Double_t fHalfSizeTPC_Z = geoPar->GetActiveRegionz() / 2.; // Z (column) [cm]
    Double_t fSizeOfVirtualPad = geoPar->GetPadSize();         // 1: pads of 1cm^2 , 5: pads of 4mm^2
    Double_t fDriftVelocity = gasPar->GetDriftVelocity();      // [cm/ns]
    Int_t NPads = 128 * 44;

    // ELECTRONICS
    Double_t channels = 4096;                                                    // 12 bits electronics
    Double_t dynamic_range = 1.2e-13;                                            // 120fC to detect particle at MIP
    Double_t ADC_Offset = 100;                                                   // To avoid negative ADC values
    Double_t ADC_conv = channels / (dynamic_range / e_equ);                      // converter from n. of e- to ADC value
    Double_t shapingtime = elecPar->GetShapingTime();                            //[ns]
    Double_t TimeBinSize = elecPar->GetTimeBinSize();                            //[ns]
    Double_t NoiseRMS = elecPar->GetNoiseRMS();                                  //[n.e-]
    Double_t thr = (elecPar->GetThreshold()) * NoiseRMS * ADC_conv + ADC_Offset; //[n.e-]
    Double_t Gain = elecPar->GetGain();
    Double_t Theta = elecPar->GetTheta();

    // Declaration of leaf types
    vector<int>* eventID = 0;
    vector<double>*x_Pad = 0, *z_Pad = 0, *t_Pad = 0, *q_Pad = 0;
    // TODO-> Define also the info regarding the vertex
    // root style
    gROOT->SetStyle("Default");
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(11);
    gStyle->SetOptFit(0);

    cout << "Reading input simulation file " << inputSimFile << endl;
    // HERE THE HISTOGRAMS ARE DEFINED...
    UInt_t histoBins = 2 * fHalfSizeTPC_X * fSizeOfVirtualPad;
    UInt_t histoBins2 = 2 * fHalfSizeTPC_Z * fSizeOfVirtualPad;
    const int max_time = (int)((2 * fHalfSizeTPC_Y) / fDriftVelocity + 2 * shapingtime);

    // Th2 definition
    TH2D* Hitmap = 0;
    TH2D* htrackInPads = 0;
    TH2D* hdriftTimeInPads = 0;
    TH2D* hdepth1InPads = 0;
    TH2D* hdepth2InPads = 0;

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
                             max_time / 10,
                             0,
                             max_time,
                             histoBins2,
                             0,
                             2 * fHalfSizeTPC_Z * fSizeOfVirtualPad);
    hdepth1InPads->SetYTitle("Z [pad number]");
    hdepth1InPads->SetXTitle("(drift) time [ns]");

    hdepth2InPads = new TH2D("hdepth2InPads",
                             "track In the Drift-X Pads Plane",
                             max_time / 10,
                             0,
                             max_time,
                             histoBins,
                             0,
                             2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
    hdepth2InPads->SetYTitle("X [pad number]");
    hdepth2InPads->SetXTitle("(drift) time [ns]");
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
    // input
    TFile* simFile = TFile::Open(inputSimFile);
    TTree* TEvt = (TTree*)simFile->Get("tout");
    // List of branches
    TEvt->SetBranchAddress("eventID", &eventID);
    TEvt->SetBranchAddress("x_Pad", &x_Pad);
    TEvt->SetBranchAddress("z_Pad", &z_Pad);
    TEvt->SetBranchAddress("t_Pad", &t_Pad);
    TEvt->SetBranchAddress("q_Pad", &q_Pad);

    // Loop over the number of events
    Int_t nentries = TEvt->GetEntries();
    cout << "N. entries= " << nentries << endl;
    if (kAllEvents)
    {
        for (unsigned int i = 0; i < nentries; i += 1)
        {
            TEvt->GetEntry(i);
            double percentage = i / (double)(nentries * 1.);
            loadfunction(percentage);
            for (unsigned int j = 0; j < x_Pad->size(); j += 1)
            {
                htrackInPads->Fill(z_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
                hdriftTimeInPads->Fill(z_Pad->at(j), x_Pad->at(j), t_Pad->at(j));
                hdepth1InPads->Fill(t_Pad->at(j), z_Pad->at(j), q_Pad->at(j));
                hdepth2InPads->Fill(t_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
                Hitmap->Fill(z_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
            }
        }
    }
    else
    {
        TEvt->GetEntry(evtnumber);
        for (unsigned int j = 0; j < x_Pad->size(); j += 1)
        {
            htrackInPads->Fill(z_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
            hdriftTimeInPads->Fill(z_Pad->at(j), x_Pad->at(j), t_Pad->at(j));
            hdepth1InPads->Fill(t_Pad->at(j), z_Pad->at(j), q_Pad->at(j));
            hdepth2InPads->Fill(t_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
            Hitmap->Fill(z_Pad->at(j), x_Pad->at(j), q_Pad->at(j));
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
    // c3_1->SetLogz();
    htrackInPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(40, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: ADC value");

    TVirtualPad* c3_2 = c3->cd(2);
    // c3_2->SetLogz();
    hdriftTimeInPads->Draw("ZCOL");

    TLatex l2;
    l2.SetTextAlign(12);
    l2.SetTextSize(0.05);
    l2.DrawLatex(40, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: drift time");

    TVirtualPad* c3_3 = c3->cd(3);
    c3_3->SetLogz();
    hdepth1InPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3 * max_time, 2.04 * fHalfSizeTPC_Z * fSizeOfVirtualPad, "Color code: ADC value");

    TVirtualPad* c3_4 = c3->cd(4);
    c3_4->SetLogz();
    hdepth2InPads->Draw("ZCOL");

    l.SetTextAlign(12);
    l.SetTextSize(0.05);
    l.DrawLatex(0.3 * max_time, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad, "Color code: ADC value");

    gPad->Modified();
    gPad->Update();

    TCanvas* c2 = new TCanvas("c2", "Hitmap", 0, 0, 1800, 1500);
    c2->SetFillColor(0);
    Hitmap->Draw("surf1");
    // c2->SetLogz();
    c2->Draw();

    // OUTPUT FILE
    c3->Print("output_readout.ps(");
    c2->Print("output_readout.ps)");
    timer.Stop();
    cout << "Macro finished succesfully!" << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
