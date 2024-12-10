
// SHOULD BE MODIFY TO INCLUDE ALL THE VERSIONS
///////////////////////////////////////////////////////////////////
//*-- AUTHOR : Hector Alvarez-Pol
//*-- Date: 04/2018
//*-- Last Update: 02/04/18
//*-- Copyright: GENP (Univ. Santiago de Compostela)
// --------------------------------------------------------------
// This macro plots the output of the glad-tpc projector:
//    - plots the R3BGTPCProjPoint which contains the virtual
//      pads calculated after the projection of the track.
// --------------------------------------------------------------
// How to run this program:
// - Run this macro inside root:
//      root[0] .L readVPadPlane.C
//      root[1] reader("inputFile")
//      root[2] guiForPads(0)
// --------------------------------------------------------------
//////////////////////////////////////////////////////////////////*/
#include <TCanvas.h>
#include <TControlBar.h>
#include <TFile.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTimer.h>

using namespace std;

const char *inputSimFile_g;
Int_t event_g = 0;
TTimer *timer;
Int_t shown = 0;

void reader(const char *inputSimFile, Int_t event = 0);

void guiForPads(Int_t firstEvent = 0) {
  // This macro shows one event and the derivative in both sides of
  // the twon ionization chamber (left and right labels have no meaning)
  //.L readTree.C;

  event_g = firstEvent;
  TControlBar *menu =
      new TControlBar("vertical", "GLADTPC Pads Viewer", 800, 650);
  menu->AddButton("      First      ", "reader(inputSimFile_g,0)",
                  "First event");
  menu->AddButton("      Go to event...      ", "reader(inputSimFile_g,-2)",
                  "Write the event number on interpreter");
  menu->AddButton("      Next       ", "reader(inputSimFile_g,event_g)",
                  "Next event");
  menu->AddButton("      Previous       ", "reader(inputSimFile_g,event_g-2)",
                  "Previous event");
  menu->AddButton("      Run continuously       ", "loop()",
                  "Press and see the events...");
  menu->AddButton("      Stop       ", "loopStop()", "Stops the exhibition");
  menu->AddButton("      See all the events   ", "reader(inputSimFile_g,-1)",
                  "See all the events added");
  menu->AddButton("      Quit       ", ".q", "Quit Root");
  gROOT->SaveContext();
  menu->Show();
  reader(inputSimFile_g, 0);
}

void loop() {
  timer = new TTimer(1000);
  timer->SetCommand("reader(inputSimFile_g,event_g)");
  timer->TurnOn();
  cout << " loop() recalled" << endl;
}

void loopStop() { timer->Stop(); }

void reader(const char *inputSimFile, Int_t event) {

  char hname[255];
  // SETUP
  string geoTag = "Prototype";
  cout << "\n \033[1;31m Warning\033[0m: the detector you are building is "
       << geoTag << "!!!!!\n"
       << endl;
  TString GTPCGeoParamsFile;
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";
  GTPCGeoParamsFile.ReplaceAll("//", "/");

  FairRuntimeDb *rtdb = FairRuntimeDb::instance();
  R3BGTPCGeoPar *geoPar = (R3BGTPCGeoPar *)rtdb->getContainer("GTPCGeoPar");
  if (!geoPar) {
    cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
    return;
  }
  R3BGTPCGasPar *gasPar = (R3BGTPCGasPar *)rtdb->getContainer("GTPCGasPar");
  if (!gasPar) {
    cout << "No R3BGTPCGasPar can be loaded from the rtdb";
    return;
  }

  FairParAsciiFileIo *parIo1 = new FairParAsciiFileIo(); // Ascii file
  parIo1->open(GTPCGeoParamsFile, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->initContainers(0);

  Double_t fHalfSizeTPC_X = geoPar->GetActiveRegionx() / 2.; // 50cm in X (row)
  Double_t fHalfSizeTPC_Y = geoPar->GetActiveRegiony() / 2.; // 20cm in Y (time)
  Double_t fHalfSizeTPC_Z =
      geoPar->GetActiveRegionz() / 2.; // 100cm in Z (column)
  Double_t fSizeOfVirtualPad =
      geoPar->GetPadSize(); // 1: pads of 1cm^2 , 10: pads of 1mm^2
  Double_t fMaxDriftTime =
      (round)((geoPar->GetActiveRegiony() / gasPar->GetDriftVelocity()) *
              pow(10, -3)); // us
  // END OF SETUP

  gROOT->SetStyle("Default");
  gStyle->SetPalette(kSolar);
  // gStyle->SetPalette(kDarkRainBow);
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  if (!shown) {
    cout << "Reading input simulation file " << inputSimFile << endl;
    cout << " for event " << event << " (- 1 means all events)." << endl;
  }
  inputSimFile_g = inputSimFile;
  event_g = event;

  // HERE THE HISTOGRAMS ARE DEFINED...
  // IT IS POSSIBLE TO CHANGE THE RANGE AND THE BINNING
  Int_t histoBins = 2 * fHalfSizeTPC_X * fSizeOfVirtualPad;
  Int_t histoBins2 = 2 * fHalfSizeTPC_Z * fSizeOfVirtualPad;
  std::cout << "READ PAD PLANE " << histoBins << " " << histoBins2 << std::endl;

  if (!shown) {
    cout << "Values taken for the visualization" << endl
         << "bins in X: " << histoBins << endl
         << "bins in Z: " << histoBins2 << endl
         << "X size (half-length of box): " << fHalfSizeTPC_X << endl
         << "Y size (half-length of box (drift)): " << fHalfSizeTPC_Y << endl
         << "Z size (half-length of box): " << fHalfSizeTPC_Z << endl
         << "Max Drift time: " << fMaxDriftTime << endl;
  }

  TH2D *htrackInPads = 0;
  TH2D *hdriftTimeInPads = 0;
  TH2D *hdepth1InPads = 0;
  TH2D *hdepth2InPads = 0;
  TH1S **h1_ProjPoint_TimeExample = 0;
  // THStack *hs = new THStack("hs","Stacked time histograms using kSolar
  // palette");

  //////////

  htrackInPads =
      new TH2D("htrackInPads", "All tracks in the XZ Pads Plane", histoBins, 0,
               histoBins, histoBins2, 0, histoBins2); // in [pad number]
  htrackInPads->SetYTitle("Z [pad number]");
  htrackInPads->SetXTitle("X [pad number]");

  hdriftTimeInPads = new TH2D("hdriftTimeInPads",
                              "All tracks in the XZ Pads Plane with drift time",
                              histoBins, 0, histoBins, histoBins2, 0,
                              histoBins2); // in [pad number]
  hdriftTimeInPads->SetYTitle("Z [pad number]");
  hdriftTimeInPads->SetXTitle("X [pad number]");

  hdepth1InPads =
      new TH2D("hdepth1InPads", "track In the Drift-Z Pads Plane", histoBins, 0,
               fMaxDriftTime, histoBins2, 0, histoBins2);
  hdepth1InPads->SetYTitle("Z [pad number]");
  hdepth1InPads->SetXTitle("(drift) time [us]");

  hdepth2InPads =
      new TH2D("hdepth2InPads", "track In the Drift-X Pads Plane", histoBins, 0,
               fMaxDriftTime, histoBins, 0, histoBins);
  hdepth2InPads->SetYTitle("X [pad number]");
  hdepth2InPads->SetXTitle("(drift) time [us]");

  TFile *simFile = TFile::Open(inputSimFile);
  TTree *TEvt = (TTree *)simFile->Get("evt");
  Int_t nevents = TEvt->GetEntries();

  if (!shown) {
    cout << "nevents=" << nevents << endl;
  }
  if (event == -2) {
    Int_t num = 0;
    cout << "Set the event number you wanna see: " << endl;
    cin >> num;
    if (num > nevents) {
      cout << "Warning: Event larger than tree size!" << endl;
    } else
      event = num;
  }

  // GTPCProjPoints
  TClonesArray *gtpcProjPointCA;
  R3BGTPCProjPoint *ppoint = new R3BGTPCProjPoint;
  gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
  TBranch *branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
  branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

  Int_t padsPerEvent = 0;
  Int_t nb = 0;
  Int_t beamPadsWithSignalPerEvent, productPadsWithSignalPerEvent;
  Double_t tPad;
  Int_t xPad, zPad, yPad;
  Int_t numberOfTimeHistos = 0;

  if (h1_ProjPoint_TimeExample)
    delete[] h1_ProjPoint_TimeExample;
  if (event != -1) { // only one event to be displayed
    gtpcProjPointCA->Clear();
    nb += TEvt->GetEvent(event);
    padsPerEvent = gtpcProjPointCA->GetEntries();
    if (padsPerEvent > 0) {
      h1_ProjPoint_TimeExample = new TH1S *[padsPerEvent];
      for (Int_t h = 0; h < padsPerEvent; h++) {
        ppoint = (R3BGTPCProjPoint *)gtpcProjPointCA->At(h);

        // xPad = ppoint->GetVirtualPadID() % (Int_t)(44);
        // zPad = (ppoint->GetVirtualPadID() - xPad) / (44);
        tPad = ((TH1S *)(ppoint->GetTimeDistribution()))->GetMean();
        htrackInPads->GetBinXYZ(ppoint->GetVirtualPadID(), xPad, zPad, yPad);
        xPad--;
        zPad--;

        htrackInPads->Fill(xPad, zPad, ppoint->GetCharge());
        hdriftTimeInPads->Fill(xPad, zPad, tPad);
        hdepth1InPads->Fill(tPad, zPad, ppoint->GetCharge());
        hdepth2InPads->Fill(tPad, xPad, ppoint->GetCharge());

        sprintf(hname, "pad %i", ppoint->GetVirtualPadID());
        h1_ProjPoint_TimeExample[h] =
            (TH1S *)((ppoint->GetTimeDistribution()))->Clone(hname);
      }

      numberOfTimeHistos = padsPerEvent;
    } else {
      event_g++;
      return; // no pads
    }
  } else { // all events
    for (Int_t i = 0; i < nevents; i++) {
      if (i % 1000 == 0)
        printf("Event:%d\n", i);
      gtpcProjPointCA->Clear();
      nb += TEvt->GetEvent(i);
      padsPerEvent = gtpcProjPointCA->GetEntries();
      if (h1_ProjPoint_TimeExample)
        delete[] h1_ProjPoint_TimeExample;
      if (padsPerEvent > 0) {
        for (Int_t h = 0; h < padsPerEvent; h++) {
          ppoint = (R3BGTPCProjPoint *)gtpcProjPointCA->At(h);

          // xPad = ppoint->GetVirtualPadID() % (Int_t)(44);
          // zPad = (ppoint->GetVirtualPadID() - xPad) / (44);
          tPad = ((TH1S *)(ppoint->GetTimeDistribution()))->GetMean();
          htrackInPads->GetBinXYZ(ppoint->GetVirtualPadID(), xPad, zPad, yPad);
          xPad--;
          zPad--;

          htrackInPads->Fill(xPad, zPad, ppoint->GetCharge());
          hdriftTimeInPads->Fill(xPad, zPad,
                                 tPad); // NOTE: THAT IS ACCUMULATED TIME!!
          hdepth1InPads->Fill(tPad, zPad, ppoint->GetCharge());
          hdepth2InPads->Fill(tPad, xPad, ppoint->GetCharge());
        }
      }
    }
  }

  TCanvas *c3 = new TCanvas("c3", "Pads in pad (XZ) plane", 0, 0, 1500, 1800);
  TLatex l;
  l.SetTextAlign(12);
  l.SetTextSize(0.05);

  c3->SetFillColor(0);
  c3->Divide(2, 2);
  c3->Draw();
  TVirtualPad *c3_1 = c3->cd(1);
  c3_1->SetLogz();
  htrackInPads->Draw("ZCOL");

  l.SetTextAlign(12);
  l.SetTextSize(0.05);
  l.DrawLatex(43.2, 45, "Color code: induced charge");

  TVirtualPad *c3_2 = c3->cd(2);
  c3_2->SetLogz();
  hdriftTimeInPads->Draw("ZCOL");

  TLatex l2;
  l2.SetTextAlign(12);
  l2.SetTextSize(0.05);
  l2.DrawLatex(43.2, 45, "Color code: drift time");

  TVirtualPad *c3_3 = c3->cd(3);
  c3_3->SetLogz();
  hdepth1InPads->Draw("ZCOL");

  l.SetTextAlign(12);
  l.SetTextSize(0.05);
  l.DrawLatex(2, 2.04 * fHalfSizeTPC_Z * fSizeOfVirtualPad,
              "Color code: induced charge");

  TVirtualPad *c3_4 = c3->cd(4);
  c3_4->SetLogz();
  hdepth2InPads->Draw("ZCOL");

  l.SetTextAlign(12);
  l.SetTextSize(0.05);
  l.DrawLatex(2, 2.04 * fHalfSizeTPC_X * fSizeOfVirtualPad,
              "Color code: induced charge");

  gPad->Modified();
  gPad->Update();

  /* if (h1_ProjPoint_TimeExample)
   {
       TVirtualPad* c3_5 = c3->cd(5);
       // c3_5->SetLogz();
       h1_ProjPoint_TimeExample[0]->SetMaximum(200);
       h1_ProjPoint_TimeExample[0]->Draw();
       // hs->Add(h1_ProjPoint_TimeExample[0]);
       // for(Int_t j=0;j<numberOfTimeHistos;j++){
       // h1_ProjPoint_TimeExample[j]->SetLineColor(kViolet+j);
       // hs->Add(h1_ProjPoint_TimeExample[j]);
       // h1_ProjPoint_TimeExample[j]->Draw("SAME");
       //}
       // hs->Draw("");

       gPad->Modified();
       gPad->Update();

       TVirtualPad* c3_6 = c3->cd(6);
       h1_ProjPoint_TimeExample[0]->SetMaximum(200);
       h1_ProjPoint_TimeExample[0]->Draw();
       for (Int_t j = 0; j < numberOfTimeHistos; j++)
       {
           h1_ProjPoint_TimeExample[j]->SetLineColor(kViolet + j);
           h1_ProjPoint_TimeExample[j]->SetFillColorAlpha(kViolet + j, 0.25);
           h1_ProjPoint_TimeExample[j]->Draw("SAME");
       }

       l.SetTextAlign(12);
       l.SetTextSize(0.05);
       l.DrawLatex(1, 50, "All pads");
   }*/

  gPad->Modified();
  gPad->Update();

  cout << " Event number " << event << " shown with " << numberOfTimeHistos
       << " projPoints. " << endl;
  event_g++;
  shown = 1;
}
