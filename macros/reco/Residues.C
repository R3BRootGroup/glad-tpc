//Macro for study the differences between GTPCPoints and GTPCHitData that we
//obtain after project with Langevin and the posterior reconstruction.
////////////////////////////////////////////////////////////////////////////////
using namespace std;

void Residues(TString simFilename = "sim.root", TString recoFilename = "output_reco.root"){
  TStopwatch timer;//To check running time of the macro
  timer.Start();

  // Input files: simulation and reconstruction -> Compare Points and Hits
  TString simFilePath;
  TString recoFilePath;

  TString workDir = gSystem->Getenv("VMCWORKDIR"); //Setting work directory
  simFilePath = workDir + "/glad-tpc/macros/sim/Prototype/" + simFilename; //File paths
  recoFilePath = workDir + "/glad-tpc/macros/reco/" + recoFilename;

  TFile* simFile = new TFile(simFilePath); //Opening Files
  TFile* recoFile = new TFile(recoFilePath);

  //Checking that opened both files
  if (simFile->IsOpen() && recoFile->IsOpen()){cout<<"[CHECK] Files correctly opened!"<<endl;}
  else{
    cout<<"[ERROR] Files not opened!"<<endl;
    exit(1);
  }

  gROOT->SetStyle("Default");
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  TCanvas *c1 = new TCanvas("Event Reconstruction", "Event Reconstruction", 0, 0, 700, 700);
  c1->SetFillColor(0);
  c1->SetFrameFillColor(0);

  //Extract info of the tree 'evt' in simFile
  //POINTS info
  TTree* simTree;
  TClonesArray* GTPCPointCA;
  TBranch* branchGTPCPoint;
  R3BGTPCPoint** points;//error: unknown type name 'R3BGTPCPoint' -> WHY?? --> Solved: create rootlogon.C
  simTree = (TTree*)simFile->Get("evt"); //Get Tree
  GTPCPointCA = new TClonesArray("R3BGTPCPoint", 5); //Create the array //What arg 5 means? Check it
  branchGTPCPoint = simTree->GetBranch("GTPCPoint"); //Get the branch of Points
  branchGTPCPoint->SetAddress(&GTPCPointCA); //GTPCPointCA ref to the branch
  //TRACKS info
  TClonesArray* MCTrackCA;
  R3BMCTrack** tracks;
  MCTrackCA = new TClonesArray("R3BMCTrack", 5);
  TBranch* branchMCTrack = simTree->GetBranch("MCTrack");
  branchMCTrack->SetAddress(&MCTrackCA);

  //Repeat for the recoFile
  TTree* recoTree;
  TClonesArray* GTPCHitDataCA;
  TBranch* branchGTPCHitData;
  R3BGTPCHitData** hits; //Posible error as in the simFile
  recoTree = (TTree*)recoFile->Get("evt"); //Get Tree
  GTPCHitDataCA = new TClonesArray("R3BGTPCHitData", 5); //Create the array //What arg 5 means? Check it
  branchGTPCHitData = recoTree->GetBranch("GTPCHitData"); //Get the branch of Hits
  branchGTPCHitData->SetAddress(&GTPCHitDataCA); //GTPCHitDataCA ref to the branch

  //Check we have same events in both files
  Long64_t sim_events = simTree->GetEntries();
  cout<<"\n[INFO] Number of events in simulation is "<<sim_events<<endl;
  Long64_t reco_events = recoTree->GetEntries();
  cout<<"\n[INFO] Number of events in recovery is "<<reco_events<<endl;

  Int_t eventPoints;
  Int_t eventHits;
  Int_t eventTracks;
  for (Int_t i=0; i<sim_events; i++){
    //Get the points produced by event i (array of R3BGTPCPoint*)
    GTPCPointCA->Clear();
    MCTrackCA->Clear();
    simTree->GetEvent(i);

    eventPoints = GTPCPointCA->GetEntries();
    eventTracks = MCTrackCA->GetEntries();
    if (i%10 == 0){
      cout<<"\n[INFO] Event ID: "<<i<<";\tNumber of Points: "<<eventPoints<<endl;
      cout<<"[INFO] Event ID: "<<i<<";\tNumber of Tracks: "<<eventTracks<<endl;

    }

    if (eventPoints > 0){
      points = new R3BGTPCPoint*[eventPoints];
      for (Int_t j=0; j<eventPoints;j++){points[j] = (R3BGTPCPoint*)GTPCPointCA->At(j);}
    }

    //Repeat for hits
    GTPCHitDataCA->Clear();
    recoTree->GetEvent(i);
    eventHits = GTPCHitDataCA->GetEntries();

    if (i%10 == 0){cout<<"[INFO] Event ID: "<<i<<";\tNumber of Hits: "<<eventHits<<endl;}

    if (eventHits > 0){
      hits = new R3BGTPCHitData*[eventHits];
      for (Int_t j=0; j<eventHits;j++) {hits[j] = (R3BGTPCHitData*)GTPCHitDataCA->At(j);}
    }

    //To Do: How can I compare Points and Hits once I can get the positions of
    //both, problem -> number of hits in an eventID can be different than then
    //number of points

    // Maybe a 3D visualization of the positions for each event?
    //It is possible or affordable?

    //3D representations of points and his ParentTrack and check the hits close
    //to the tracks a minimum distance to be considereded reconstruction of this track

    //Starting studying only one event
    if (i==990){
      Double_t x;
      Double_t y;
      Double_t z;
      Double_t trackID;
      //Create the graphs
      TGraph2D *hitgr  = new TGraph2D(eventHits);
      TGraph2D *pointgr = new TGraph2D(eventPoints);

      for (Int_t j=0; j<eventHits; j++){
        x = hits[j]->GetX();
        y = hits[j]->GetY();
        z = hits[j]->GetZ();
        hitgr->SetPoint(j,x,y,z);
      }
      //Drawing the 3D graphs
      hitgr->SetMarkerStyle(4);
      hitgr->SetMarkerColor(2);
      hitgr->Draw("PO");

      for (Int_t j=0; j<eventPoints;j++){
        x = points[j]->GetX();
        y = points[j]->GetY();
        z = points[j]->GetZ();
        pointgr->SetPoint(j,x,y,z);
        trackID = points[j]->GetTrackID();
        cout<<"Point: "<<j<<"\tTrackID: "<<trackID<<"\tPosition: ("<<x<<" "<<y<<" "<<z<<")"<<endl;
      }
      pointgr->SetMarkerStyle(4);
      pointgr->SetMarkerColor(4);
      pointgr->Draw("same LINE");
      pointgr->Draw("same P");

    }

    //Delete arrays
    if (eventPoints)
      delete[] points;
    if (eventHits)
      delete[] hits;
  }

  //Ending the macro
  timer.Stop();
  cout<<"\n[INFO] Macro finished succesfully!"<<endl;
  cout<<"Real time: "<<timer.RealTime()<<"s; CPU time: "<<timer.CpuTime()<<"s"<<endl;
}
