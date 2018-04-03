//  -------------------------------------------------------------------------
//
//   ----- General Macro for checking the projPoints from the projector
//         Author: Hector Alvarez <hector.alvarez@usc.es>
//         Comments:
//			Check the projPoints.
//  -------------------------------------------------------------------------
//
//   Usage:
//      > root -l readProjPoints.C
//
//     BUT FIRST, select in the //SETTINGS section the simulation features
//	(the macro will plot and text information as a function of these settings)
//  -------------------------------------------------------------------------
void readProjPoints() {

  char title1[250];  char hname[255];
  //SETTINGS
  Bool_t graphicalOutput = kTRUE;
  Bool_t textOutput = kFALSE;
  Bool_t checkProjPoints=kTRUE; //Defines Histogram for ProjPoints
  Bool_t visualizeVPadPlane=kTRUE; //Defines VpadPlane viewer
  
  sprintf(title1,"%s","proj.root"); //INPUT FILE 
  TFile *file1 = TFile::Open(title1);
  //END OF THE SETTING AREA
  
  gROOT->SetStyle("Default");
  gStyle->SetOptStat(1111111);
  gStyle->SetOptFit(1);
  
  //HISTOGRAMS DEFINITION
  TH1F *h1_ProjPoint_Mult, *h1_ProjPoint_Charge, *h1_ProjPoint_VirtualPadID; 
  TH2F *h2_ProjPoint_VPadPlane;
  TH1S **h1_ProjPoint_TimeExample;
  if(graphicalOutput && checkProjPoints){  //HISTOGRAMS DEFINITION FOR PROJPOINTS
    h1_ProjPoint_Mult = new TH1F("h1_ProjPoint_Mult","ProjPoint Multiplicity",400,0,400);
    h1_ProjPoint_Charge = new TH1F("h1_ProjPoint_Charge","ProjPoint Charge",1000,0,1000);
    h1_ProjPoint_VirtualPadID = new TH1F("h1_ProjPoint_VirtualPadID","ProjPoint VirtualPadID",5200,0,5200);
    h2_ProjPoint_VPadPlane= new TH2F("h2_ProjPoint_VPadPlane","ProjPoint VPadPlane",
				     100,-50,50,1000,200,300);
    //h1_ProjPoint_TimeExample = new TH1F("h1_ProjPoint_TimeExample","ProjPoint TimeExample",100,0,100);
  }
  //END OF HISTOGRMAS DEFINITION
  TTree* TEvt = (TTree*)file1->Get("evt");

  //GTPCProjPoints
  TClonesArray* gtpcProjPointCA;
  R3BGTPCProjPoint** ppoint;
  gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint",5);
  TBranch *branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
  branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

  Long64_t nevents = TEvt->GetEntries();
  if(textOutput) cout << "INFO:  The number of events is "<< nevents << endl;

  Int_t ppointsPerEvent = 0;
  Int_t numberOfTimeHistos = 0;

  for(Int_t i=0;i<nevents;i++){
    if(i%10000 == 0) printf("Event:%i\n",i);
    
    gtpcProjPointCA->Clear();

    TEvt->GetEvent(i);

    ppointsPerEvent = gtpcProjPointCA->GetEntries();
    if(textOutput){
      cout << endl <<"INFO:  The number of projPoints in this event is "
	   << ppointsPerEvent << endl;
    }
    
    if(ppointsPerEvent>0) {
      ppoint = new R3BGTPCProjPoint*[ppointsPerEvent];
      for(Int_t j=0;j<ppointsPerEvent;j++){
	ppoint[j] = new R3BGTPCProjPoint;
	ppoint[j] = (R3BGTPCProjPoint*) gtpcProjPointCA->At(j);
      }
    }
    
    if (i==0 && (ppointsPerEvent>0)) { 
      //only for the first event, show time distributions in a single pad
      h1_ProjPoint_TimeExample = new TH1S*[ppointsPerEvent];
      for(Int_t j=0;j<ppointsPerEvent;j++){
	//h1_ProjPoint_TimeExample[j] = new TH1S(ppoint[j]->GetTimeDistribution());
	//h1_ProjPoint_TimeExample[j] = ppoint[j]->GetTimeDistribution();
	sprintf(hname,"pad %i",ppoint[j]->GetVirtualPadID());
	h1_ProjPoint_TimeExample[j] = (TH1S*)((ppoint[j]->GetTimeDistribution()))->Clone(hname);
      }
      numberOfTimeHistos=ppointsPerEvent;
    }
    
    //GTPC Point information
    if(checkProjPoints){	  
      if(graphicalOutput) h1_ProjPoint_Mult->Fill(ppointsPerEvent);
      for(Int_t h=0;h<ppointsPerEvent;h++){
	if(textOutput) {
	  cout << "ProjPoint number " << h << endl; 
	  ppoint[h]->Print();
	}
	if(graphicalOutput) {
	  h1_ProjPoint_Charge->Fill(ppoint[h]->GetCharge());
	  h1_ProjPoint_VirtualPadID->Fill(ppoint[h]->GetVirtualPadID());
	}
      }
    }

    if(ppointsPerEvent) delete[] ppoint;
  }


  if(graphicalOutput){
    TCanvas* c1 = new TCanvas("ProjPoints","ProjPoints",20,20,620,720);
    c1->SetFillColor(0);
    c1->SetFrameFillColor(0);
    c1->cd();
    c1->Divide(2,2);
    c1->cd(1); c1->cd(1)->SetLogy(); h1_ProjPoint_Mult->Draw();
    c1->cd(2); c1->cd(2)->SetLogy(); h1_ProjPoint_Charge->Draw();
    c1->cd(3); h1_ProjPoint_VirtualPadID->Draw();
    c1->cd(4); h1_ProjPoint_TimeExample[0]->GetXaxis()->SetRange(0,50);
    h1_ProjPoint_TimeExample[0]->Draw();
    for(Int_t j=1;j<2;j++){
      h1_ProjPoint_TimeExample[j]->SetLineColor(kRed);
      h1_ProjPoint_TimeExample[j]->Draw("SAME");
    }

    //OUTPUT FILE
    //ctext->Print("output.ps(");
    c1->Print("output.ps");
    //c2->Print("output.ps");
    //c3->Print("output.ps)");
  }
}
