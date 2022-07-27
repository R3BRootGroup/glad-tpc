//Macro for study the differences between GTPCPoints and GTPCHitData that we
//obtain after project with Langevin and the posterior reconstruction.
////////////////////////////////////////////////////////////////////////////////
using namespace std;

Double_t GetDistance(Double_t x1, Double_t z1, Double_t x2, Double_t z2){
  Double_t distance;
  distance = (Double_t) TMath::Sqrt((x1-x2)*(x1-x2)+(z1-z2)*(z1-z2));
  return distance;
}

Double_t GetMinDistance(Double_t x1, Double_t z1, Double_t* x, Double_t* z){
  Double_t min_dist = GetDistance(x1,z1,x[0],z[0]);
  Double_t distance;
  for (Int_t i=0; i < sizeof(x); i++){
    distance = GetDistance(x1,z1,x[i],z[i]);
    if (distance < min_dist) {min_dist = distance;}
  }
  return min_dist;
}

void residues(TString simFilename = "sim.root", TString recoFilename = "output_reco.root"){
  //Timer for runtime check
  TStopwatch timer;
  timer.Start();
  // Input files: simulation and reconstruction -> Compare Points and Hits
  TString simFilePath;
  TString recoFilePath;
  //Setting up env and paths
  TString workDir = gSystem->Getenv("VMCWORKDIR"); //Setting work directory
  simFilePath = workDir + "/glad-tpc/macros/sim/Prototype/" + simFilename; //File paths
  recoFilePath = workDir + "/glad-tpc/macros/reco/" + recoFilename;
  //Opening Files
  TFile* simFile = new TFile(simFilePath);
  TFile* recoFile = new TFile(recoFilePath);
  //Checking that opened both files
  if (simFile->IsOpen() && recoFile->IsOpen()){cout<<"[INFO] Files correctly opened!"<<endl;}
  else {cout<<"[ERROR] Files not opened!"<<endl; exit(1);}

  // ------------------- Extracting info of the TREE ---------------------------
  //POINTS TREE
  TTree* simTree;
  TClonesArray* GTPCPointCA;
  TBranch* branchGTPCPoint;
  R3BGTPCPoint** points;//error: unknown type name 'R3BGTPCPoint' -> WHY?? --> Solved: create rootlogon.C
  simTree = (TTree*)simFile->Get("evt"); //Get Tree
  GTPCPointCA = new TClonesArray("R3BGTPCPoint", 5); //Create the array //What arg 5 means? Check it
  branchGTPCPoint = simTree->GetBranch("GTPCPoint"); //Get the branch of Points
  branchGTPCPoint->SetAddress(&GTPCPointCA); //GTPCPointCA ref to the branch

  //TRACKS INFO
  TClonesArray* MCTrackCA;
  R3BMCTrack** tracks;
  MCTrackCA = new TClonesArray("R3BMCTrack", 5);
  TBranch* branchMCTrack = simTree->GetBranch("MCTrack");
  branchMCTrack->SetAddress(&MCTrackCA);

  //HITS INFO
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
  cout<<"[INFO] Number of events in simulation is "<<sim_events<<endl;
  Long64_t reco_events = recoTree->GetEntries();
  cout<<"[INFO] Number of events in reconstruction is "<<reco_events<<endl;

  //Canvas and histo fro visualizing residues
  TCanvas* c = new TCanvas("c","XZ Residues",0,500,500,500);
  TH1D* reshist = new TH1D("reshist", "Residues Histogram", 501,0,10);

  //Integers variables to store the number of different data in each event
  Int_t eventPoints;
  Int_t eventHits;
  Int_t eventTracks;
  //Main loop through all events
  for (Int_t i=0; i<sim_events; i++){
    //Getting the points in event i
    GTPCPointCA->Clear();
    MCTrackCA->Clear();
    simTree->GetEvent(i);
    eventPoints = GTPCPointCA->GetEntries();
    eventTracks = MCTrackCA->GetEntries();
    //Getting the hits in event i
    GTPCHitDataCA->Clear();
    recoTree->GetEvent(i);
    eventHits = GTPCHitDataCA->GetEntries();

    //Printing some info
    /*if (i%10 == 0){
      cout<<"\n[INFO] Event ID: "<<i<<";\tNumber of Points: "<<eventPoints<<endl;
      cout<<"[INFO] Event ID: "<<i<<";\tNumber of Tracks: "<<eventTracks<<endl;
      cout<<"[INFO] Event ID: "<<i<<";\tNumber of Hits: "<<eventHits<<endl;

    }*/

    //Create points and hits arrays and fill them
    if (eventPoints > 0){
      points = new R3BGTPCPoint*[eventPoints];
      for (Int_t j=0; j<eventPoints;j++){points[j] = (R3BGTPCPoint*)GTPCPointCA->At(j);}
    }
    if (eventHits > 0){
      hits = new R3BGTPCHitData*[eventHits];
      for (Int_t j=0; j<eventHits;j++) {hits[j] = (R3BGTPCHitData*)GTPCHitDataCA->At(j);}
    }

    //One particular event
    if (i==999){
      //Creating the canvas
      TCanvas* c1 = new TCanvas("c1","Event 2D",0,0,500,500);
      TCanvas* c2 = new TCanvas("c2","Event 3D",500,0,500,500);
      TCanvas* c3 = new TCanvas("c3","Event Residues",1000,0,500,500);

      //Create variables
      Double_t x;
      Double_t y;
      Double_t z;

      Int_t trackID;
      Int_t tempID = 0;
      Int_t k = 0;
      //Create the graphs 3d
      TGraph2D* hitgr3d  = new TGraph2D(eventHits);
      TGraph2D* pointgr3d = new TGraph2D(eventPoints);
      TGraph2D* trackgr3d = new TGraph2D();
      //Create 2D graphs
      TGraph* fitgr = new TGraph(); //Graph to plot take the primary track points to be fitted
      TGraph* hitgr  = new TGraph(eventHits);
      TGraph* pointgr = new TGraph(eventPoints);
      TF1* fit = new TF1("fit", "pol2", 5, 15);// x -> [5,15]cm

      TH1D* res = new TH1D("res", "Residues", 501, 0, 10);

      //Loop through points in event
      for (Int_t j=0; j<eventPoints;j++){
        x = points[j]->GetX(); y = points[j]->GetY(); z = points[j]->GetZ();
        //Filling points in graph
        pointgr3d->SetPoint(j,x,z,y); //WARNING! We are putting our z in y axis
        pointgr->SetPoint(j,x,z);
        trackID = points[j]->GetTrackID();
        //cout<<"Point: "<<j<<"\tTrackID: "<<trackID<<"\tPosition: ("<<x<<" "<<y<<" "<<z<<")"<<endl;

        // Prepare Primary Track visualization
        if (trackID == tempID){
          trackgr3d->SetPoint(k,x,z,y);
          fitgr->SetPoint(k,x,z);
          k ++;
        }
        /*else{//Draw all the secondary tracks
          trackgr->Draw("same LINE");
          trackgr = new TGraph2D();
          trackgr->SetLineColor(trackID); //Different color for non primary tracks
          trackgr->Set(1);
          trackgr->SetPoint(0,x,y,z);
          tempID = trackID;
          k = 1;
        }*/
      }
      cout<<"[INFO] Fitting... Event "<<i<<endl;
      c1->cd();
      fitgr->Fit("fit"); //We have the fitted function
      fit->Draw("");

      //Creating points following the fit to calculate distances
      Int_t n_fitpoints = 200;
      Double_t fit_x[n_fitpoints];
      Double_t fit_z[n_fitpoints];
      TRandom* r3 = new TRandom3();
      Double_t p;
      for (Int_t j=0; j<n_fitpoints; j++){
        p = r3->Rndm(j);
        fit_x[j] = p*10+5;
        fit_z[j] = fit->Eval(fit_x[j]);
        //cout<<fit_x[j]<<"  "<<fit_z[j]<<endl;
      }


      Double_t energy;
      Double_t distance;
      for (Int_t j=0; j<eventHits; j++){
        x = hits[j]->GetX(); y = hits[j]->GetY(); z = hits[j]->GetZ();
        //We could get the enegy that we can use as a weight to make ponderate residuals
        //Get Distance to primitive to get residuals; create the function pol2 with fitted params
        //Show residuals in a new canvas
        hitgr3d->SetPoint(j,x,z,y);
        hitgr->SetPoint(j,x,z);
        energy = hits[j]->GetEnergy();
        distance = GetMinDistance(x,z,fit_x,fit_z);
        //cout<<distance<<endl;
        res->Fill(distance, energy);

      }
      c1->cd();
      pointgr->SetMarkerStyle(4);
      pointgr->SetMarkerSize(1);
      pointgr->SetMarkerColor(4);
      pointgr->Draw("same P");

      hitgr->SetMarkerStyle(4);
      hitgr->SetMarkerSize(0.2);
      hitgr->SetMarkerColor(2);
      hitgr->Draw("same P");

      c2->cd();
      //Drawing the 3D graphs
      hitgr3d->SetMarkerStyle(4);
      hitgr3d->SetMarkerSize(0.2);
      hitgr3d->SetMarkerColor(2);
      hitgr3d->Draw("P");

      trackgr3d->Draw("same LINE");

      pointgr3d->SetMarkerStyle(4);
      pointgr3d->SetMarkerSize(0.2);
      pointgr3d->SetMarkerColor(4);
      pointgr3d->Draw("same P");

      c3->cd();
      res->Draw("HIST");
    }


    //Create variables
    Double_t x;
    Double_t y;
    Double_t z;

    Int_t trackID;
    Int_t tempID = 0;
    Int_t k = 0;

    TGraph* fitgr = new TGraph(); //Graph to plot take the primary track points to be fitted

    TF1* fit = new TF1("fit", "pol2", 5, 15);// x -> [5,15]cm


    //Loop through points in event
    for (Int_t j=0; j<eventPoints;j++){
      x = points[j]->GetX(); y = points[j]->GetY(); z = points[j]->GetZ();
      trackID = points[j]->GetTrackID();
      if (trackID == tempID){
        fitgr->SetPoint(k,x,z);
        k ++;
      }
    }
    cout<<"\n[INFO] Fitting... Event "<<i;
    fitgr->Fit("fit"); //We have the fitted function

    //Creating points following the fit to calculate distances
    Int_t n_fitpoints = 200;
    Double_t fit_x[n_fitpoints];
    Double_t fit_z[n_fitpoints];
    TRandom* r3 = new TRandom3();
    Double_t p;
    for (Int_t j=0; j<n_fitpoints; j++){
      p = r3->Rndm(j);
      fit_x[j] = p*10+5;
      fit_z[j] = fit->Eval(fit_x[j]);
      //cout<<fit_x[j]<<"  "<<fit_z[j]<<endl;
    }


    Double_t energy;
    Double_t distance;
    for (Int_t j=0; j<eventHits; j++){
      x = hits[j]->GetX(); y = hits[j]->GetY(); z = hits[j]->GetZ();
      energy = hits[j]->GetEnergy();
      distance = GetMinDistance(x,z,fit_x,fit_z);
      reshist->Fill(distance, energy);

    }

    //Event i finishes ==> Deleting the arrays
    if (eventPoints)
      delete[] points;
    if (eventHits)
      delete[] hits;
  }
  c->cd();
  reshist->Draw("HIST");

  //Ending the macro
  timer.Stop();
  cout<<"\n[INFO] Macro finished succesfully !!"<<endl;
  cout<<"Real time: "<<timer.RealTime()<<"s; CPU time: "<<timer.CpuTime()<<"s"<<endl;
}
