#include "fitter.h"

#include "R3BGTPCFitter.h"

int main(int argc, char *argv[])
{

  Bool_t  fInteractiveMode = 1;
  Double_t M_Ener = 0.139570;//Pion mass in GeV

  
  //Histograms
  TH1F *momentum = new TH1F("momentum", "momentum", 1000, 0, 2.0);
  TH1F *energy = new TH1F("energy", "energy", 1000, 0, 2.0);
  
   // Paths
   TString dir = getenv("VMCWORKDIR");

   TString geoManFile = "/mnt/simulations/attpcroot/fair_install_2020/R3BRoot/glad-tpc/geometry/HYDRA_Prototype.geoMan.root";
   

   std::cout << " Geometry file : " << geoManFile.Data() << "\n";


   TString fileName = "/mnt/simulations/attpcroot/fair_install_2020/R3BRoot/glad-tpc/macros/tracking/output_tracking.root";
   TFile* file = new TFile(fileName.Data(), "READ");
   
   
   // GENFIT geometry
   new TGeoManager("Geometry", "HYDRA geometry");
   TGeoManager::Import(geoManFile.Data());
   genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
   genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.0,15.0,0.0)); //

    // event display
   genfit::EventDisplay *display;
   if (fInteractiveMode)
      display = genfit::EventDisplay::getInstance();

   R3BGTPCFitter *fitter = new R3BGTPCFitter();

   TTree *tree = (TTree *)file->Get("evt");
   Int_t nEvents = tree->GetEntries();
   std::cout << " Number of events : " << nEvents << std::endl;

    TTreeReader Reader1("evt", file);
    TTreeReaderValue<TClonesArray> trackArray(Reader1, "GTPCTrackData");
    //Reader1.SetEntriesRange(0, nEvents);

  for (Int_t i = 0; i < nEvents; i++) {

    //Reset variables

    std::cout << cGREEN << " ------ Event Number : " << i << cNORMAL << "\n";

    Reader1.Next();

    Int_t nTracks = trackArray->GetEntries();
    std::cout<<" Number of tracks "<<nTracks<<"\n";

    R3BGTPCTrackData** trackData;
    trackData = new R3BGTPCTrackData*[nTracks];

    for(auto iTrack = 0;iTrack<nTracks;++iTrack)
    {
       trackData[iTrack] = (R3BGTPCTrackData*)(trackArray->At(iTrack));

       fitter->Init();
       genfit::Track *fitTrack;
       
       fitTrack = fitter->FitTrack(trackData[iTrack]);

       TVector3 pos_res;
       TVector3 mom_res;
       TMatrixDSym cov_res;
       Double_t pVal = 0;
       Double_t bChi2 = 0, fChi2 = 0, bNdf = 0, fNdf = 0;
               

       try {

                  if (fitTrack && fitTrack->hasKalmanFitStatus()) {

                     auto KalmanFitStatus = fitTrack->getKalmanFitStatus();
                     auto trackRep = fitTrack->getTrackRep(0); // Only one representation is sved for the moment.

                     
                     if (KalmanFitStatus->isFitConverged(false)) {
                        // KalmanFitStatus->Print();
                        genfit::MeasuredStateOnPlane fitState = fitTrack->getFittedState();			
                        // fitState.Print();
                        fitState.getPosMomCov(pos_res, mom_res, cov_res);
			fChi2 = KalmanFitStatus->getForwardChi2();
                        bChi2 = KalmanFitStatus->getBackwardChi2();
                        fNdf  = KalmanFitStatus->getForwardNdf();
                        bNdf  = KalmanFitStatus->getBackwardNdf();

			if (fInteractiveMode)
			  display->addEvent(fitTrack);

			momentum->Fill(mom_res.Mag());
			Double_t E = TMath::Sqrt(TMath::Power(mom_res.Mag(), 2) + TMath::Power(M_Ener, 2)) - M_Ener;
			energy->Fill(E);
			
		     } // Fit converged
                  }
		  
               } catch (std::exception &e) {
                  std::cout << " " << e.what() << "\n";
                  continue;
               }
    }//Tracks
    
     if (trackData)
        delete trackData;
    
    

  }

   
   
  
   if (fInteractiveMode) {

     TCanvas *c1 = new TCanvas();
      c1->Divide(1, 2);
      c1->Draw();
      c1->cd(1);
      momentum->Draw();
      c1->cd(2);
      energy->Draw();


     // open event display
      display->open();

      
   } // Interactive mode
      
  return 0;
}
