#include "fitter.h"

#include "R3BGTPCFitter.h"

int main(int argc, char *argv[])
{

  Bool_t  fInteractiveMode = 0;
     
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
   genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.0,5.0,0.0)); //

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
    
       fitter->FitTrack(trackData[iTrack]);

    }
    
     if (trackData)
        delete trackData;
    
    fitter->Init();

  }

   
  
   if (fInteractiveMode) {
      // open event display
      display->open();

   } // Interactive mode
   
  return 0;
}
