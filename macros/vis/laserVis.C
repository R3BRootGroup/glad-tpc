/*

This macro is a different version of readVPadPlane.C, modiffied to
show the results of the laser generation.

*/

void laserVis(TString inputSimFile, TString title, TString mode) {

  // Pad plane
  std::shared_ptr<R3BGTPCMap> fTPCMap;
  TH2 *fPadPlane;
  fTPCMap = std::make_shared<R3BGTPCMap>();
  fTPCMap->GeneratePadPlane();
  fPadPlane = fTPCMap->GetPadPlane();

  ////////////////////////////////////////////////////////////////////////
  // EVENT ACCESS
  ////////////////////////////////////////////////////////////////////////

  // File access
  TFile *simFile = TFile::Open(inputSimFile, "READ");
  TTree *TEvt = (TTree *)simFile->Get("evt");
  Int_t nevents = TEvt->GetEntries();

  // Projection Point Definition
  TClonesArray *gtpcProjPointCA;
  R3BGTPCProjPoint *ppoint = new R3BGTPCProjPoint;
  gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
  TBranch *branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
  branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

  // Event 0 analysis
  Int_t padsPerEvent = 0;
  Int_t xPad, zPad;

  gtpcProjPointCA->Clear();
  TEvt->GetEvent(0);
  padsPerEvent = gtpcProjPointCA->GetEntries();

  if (padsPerEvent > 0) {

    for (Int_t h = 0; h < padsPerEvent; h++) {
      ppoint = (R3BGTPCProjPoint *)gtpcProjPointCA->At(h);
      if (ppoint->GetVirtualPadID() == -1) {
        continue;
      }

      xPad = fTPCMap->CalcPadCenter(ppoint->GetVirtualPadID())[1];
      zPad = fTPCMap->CalcPadCenter(ppoint->GetVirtualPadID())[0];

      // xPad = ppoint->GetVirtualPadID() % 44;
      // zPad = (ppoint->GetVirtualPadID() - xPad) / 44.;
      fPadPlane->Fill(zPad, xPad, ppoint->GetCharge());
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // RESULTS
  ////////////////////////////////////////////////////////////////////////

  auto *file = new TFile("laser_results.root", mode);
  auto *c = new TCanvas("", "", 8 * 128, 8 * 44);
  gStyle->SetPalette(112);
  c->SetLogz();
  gStyle->SetOptStat(0);
  fPadPlane->Draw("zcol L");
  fPadPlane->Write();
  file->Close();
}
