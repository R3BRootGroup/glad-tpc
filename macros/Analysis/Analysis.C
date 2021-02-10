//  -------------------------------------------------------------------------
//
//   ----- General Macro for check and analize all results
//         Author: Simone Velardita
//  -------------------------------------------------------------------------
//
//   Usage: Modify the GEOTAG and version accordingly to waht you want and then
//      > root -l Analysis.C
//
//  -------------------------------------------------------------------------
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

void Analysis(TString GEOTAG="FullBeamIn", TString version="v2")
{
		 // -----   Timer   --------------------------------------------------------
		 TStopwatch timer;
		 timer.Start();

    // SETTINGS
    Bool_t graphicalOutput = kTRUE;
    Bool_t textOutput = kFALSE;

    Bool_t checkMCTracks = kTRUE; // Defines Histogram for MCTracks
    Bool_t checkPoints = kTRUE;   // Defines Histogram for Points
    TString inFile;
    // Input file according to the GEOTAG
    if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31mWarning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/Prototype/sim.root";
    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/FullBeamOut/sim.root";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG<<"."<<version<< endl;
        if (version.CompareTo("v1")==0) 
        {
				    inFile = "../sim/FullBeamIn/v1/sim.root";
        }
    		if (version.CompareTo("v2")==0)
    		{
				    inFile = "../sim/FullBeamIn/v2/sim.root";
        }
    		if (version.CompareTo("v3")==0) 
    		{
				    inFile = "../sim/FullBeamIn/v3/sim.root";
        }
    }

    // Opening the input
    TFile* file1 = new TFile(inFile);
    if (file1->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);

    Double_t maxE = 100; // max energy in plots

    gDebug = 0; //    Debug option
    // END OF THE SETTING AREA

    gROOT->SetStyle("Default");
    gStyle->SetOptStat(111111);
    gStyle->SetOptFit(1);

    // HISTOGRAMS DEFINITION
    TH1F *h1_MC_Mult, *h1_MC_PDG, *h1_MC_Ene, *h1_MC_Theta, *h1_MC_Phi;
    TH1F *h1_MC_MultSec, *h1_MC_PDGSec, *h1_MC_EneSec, *h1_MC_ThetaSec, *h1_MC_PhiSec;
    TH1F *h1_Point_Mult, *h1_Point_Time, *h1_Point_Length, *h1_Point_ELoss;
    TH1F *h1_Point_TrackStatus, *h1_Point_PDG, *h1_Point_Charge, *h1_Point_Mass;
    TH1F *h1_Point_Kine, *h1_Point_trackStep;
    TH1D *h1_Pions_TrackLength, *h1_Pions_ELoss;
    TH2F *h2_Point_XZ, *h2_Point_YZ;
    TH2F *h2_Point_PxPz, *h2_Point_PyPz;
    if (graphicalOutput && checkMCTracks)
    { // HISTOGRAMS DEFINITION FOR MCTRACKS
        h1_MC_Mult = new TH1F("h1_MC_Mult", "MCTrack Multiplicity (primaries)", 100, 0, 10);
        // if secondaries are included
        h1_MC_MultSec = new TH1F("h1_MC_MultSec", "MCTrack Multiplicity", 100, 0, 1000);
        h1_MC_PDG = new TH1F("h1_MC_PDG", "Primary PDG Code", 4600, -2300, 2300);
        h1_MC_PDGSec = new TH1F("h1_MC_PDGSec", "MCTrack PDG Code", 4600, -2300, 2300);
        h1_MC_Ene = new TH1F("h1_MC_Ene", "Primary Energy (MeV)", 200, 0, 3 * maxE);
        h1_MC_EneSec = new TH1F("h1_MC_EneSec", "MCTrack Energy (MeV)", 200, 0, 3 * maxE);
        h1_MC_Theta = new TH1F("h1_MC_Theta", "Primary Theta", 200, 0, 3.2);
        h1_MC_ThetaSec = new TH1F("h1_MC_ThetaSec", "MCTrack Theta", 200, 0, 3.2);
        h1_MC_Phi = new TH1F("h1_MC_Phi", "Primary Phi", 200, -3.2, 3.2);
        h1_MC_PhiSec = new TH1F("h1_MC_PhiSec", "MCTrack Phi", 200, -3.2, 3.2);
    }
    if (graphicalOutput && checkPoints)
    { // HISTOGRAMS DEFINITION FOR POINTS
        h1_Point_Mult = new TH1F("h1_Point_Mult", "Point Multiplicity", 400, 0, 400);
        h1_Point_Time = new TH1F("h1_Point_Time", "Point Time", 400, 0, 0.0000001);
        h1_Point_Length = new TH1F("h1_Point_Length", "Point Length", 400, 0, 400);
        h1_Point_ELoss = new TH1F("h1_Point_ELoss", "Point ELoss (MeV)", 400, 0, 0.1);
        h1_Point_TrackStatus = new TH1F("h1_Point_TrackStatus", "Point TrackStatus", 8, 0, 7);
        h1_Point_PDG = new TH1F("h1_Point_PDG", "Point PDG", 4600, -2300, 2300);
        h1_Point_Charge = new TH1F("h1_Point_Charge", "Point Charge", 40, 0, 39);
        h1_Point_Mass = new TH1F("h1_Point_Mass", "Point Mass", 400, 0, 10);
        h1_Point_Kine = new TH1F("h1_Point_Kine", "Point KinE (MeV)", 400, 0, 6000);
        h1_Point_trackStep = new TH1F("h1_Point_trackStep", "Point trackStep", 400, 0, 40);
        h1_Pions_TrackLength = new TH1D("h1_Pions_TrackLength", "Pions trackLength (cm)", 100, 0, 100);
        h1_Pions_ELoss = new TH1D("h1_Pions_ELoss", "Pions ELoss (MeV)", 900, 0, 0.2);
        //h2_Point_XZ = new TH2F("h2_Point_XZ", "Points projection on XZ plane", 600, -30, 30, 1100, 195, 305);
        //h2_Point_YZ = new TH2F("h2_Point_YZ", "Point projection on YZ plane", 600, -30, 30, 1100, 195, 305);
        h2_Point_XZ = new TH2F("h2_Point_XZ", "Points projection on XZ plane", 1200, 175, 295, 600, -30, 30);
        h2_Point_YZ = new TH2F("h2_Point_YZ", "Point projection on YZ plane", 1200, 175, 295, 600, -30, 30);        
        h2_Point_PxPz = new TH2F("h2_Point_PxPz", "Momentum projection on XZ plane", 600, -1, 1, 600, -1, 7);
        h2_Point_PyPz = new TH2F("h2_Point_PyPz", "Momentum projection on YZ plane", 600, -1, 1, 600, -1, 7);
    }
    // END OF HISTOGRMAS DEFINITION

    TTree* TEvt = (TTree*)file1->Get("evt");

    // GTPCPoints
    TClonesArray* gtpcPointCA;
    R3BGTPCPoint** point;
    gtpcPointCA = new TClonesArray("R3BGTPCPoint", 5);
    TBranch* branchGTPCPoint = TEvt->GetBranch("GTPCPoint");
    branchGTPCPoint->SetAddress(&gtpcPointCA);

    // MCTrack(input)
    TClonesArray* MCTrackCA;
    R3BMCTrack** track;
    MCTrackCA = new TClonesArray("R3BMCTrack", 5);
    TBranch* branchMCTrack = TEvt->GetBranch("MCTrack");
    branchMCTrack->SetAddress(&MCTrackCA);

    Long64_t nevents = TEvt->GetEntries();
    if (textOutput)
        cout << "INFO:  The number of events is " << nevents << endl;

    ROOT::Math::PxPyPzMVector momentum;
    Int_t pointsPerEvent = 0;
    Int_t MCtracksPerEvent = 0;
    Int_t primaries = 0;

    for (Int_t i = 0; i < nevents; i++)
    {
        double percentage = i / (double)(nevents * 1.);
        loadfunction(percentage);
        //cout<<"n. event="<<i<<endl;
        gtpcPointCA->Clear();
        MCTrackCA->Clear();
        primaries = 0;

        TEvt->GetEvent(i);

        pointsPerEvent = gtpcPointCA->GetEntries();
        MCtracksPerEvent = MCTrackCA->GetEntries();
        if (textOutput)
        {
            cout << endl << "INFO:  The number of points in this event is " << pointsPerEvent << endl;
            cout << "INFO:  The number of MCtracks in this event is " << MCtracksPerEvent << endl;
        }

        if (pointsPerEvent > 0)
        {
            point = new R3BGTPCPoint*[pointsPerEvent];
            for (Int_t j = 0; j < pointsPerEvent; j++)
            {
                point[j] = new R3BGTPCPoint;
                point[j] = (R3BGTPCPoint*)gtpcPointCA->At(j);
            }
        }
        if (MCtracksPerEvent > 0)
        {
            track = new R3BMCTrack*[MCtracksPerEvent];
            for (Int_t j = 0; j < MCtracksPerEvent; j++)
            {
                track[j] = new R3BMCTrack;
                track[j] = (R3BMCTrack*)MCTrackCA->At(j);
            }
        }
     double zz=0;
        // loop in MC mother tracks
        if (checkMCTracks)
        {
            h1_MC_MultSec->Fill(MCtracksPerEvent);

            for (Int_t h = 0; h < MCtracksPerEvent; h++)
            {
                if (track[h]->GetMotherId() < 0)
                {
                    primaries++;
                    momentum = track[h]->GetMomentumMass();
                    if (graphicalOutput)
                    {
                        h1_MC_PDG->Fill(track[h]->GetPdgCode());
                        h1_MC_Ene->Fill(track[h]->GetEnergy() * 1000 - track[h]->GetMass() * 1000);
                        h1_MC_Theta->Fill(momentum.Theta());
                        h1_MC_Phi->Fill(momentum.Phi());
                    }
                    if(track[h]->GetPdgCode()==-211) zz=track[h]->GetStartZ();
                }
                else
                {
                    momentum = track[h]->GetMomentumMass();
                    h1_MC_PDGSec->Fill(track[h]->GetPdgCode());
                    h1_MC_EneSec->Fill(track[h]->GetEnergy() * 1000 - track[h]->GetMass() * 1000);
                    h1_MC_ThetaSec->Fill(momentum.Theta());
                    h1_MC_PhiSec->Fill(momentum.Phi());
                }
            }
            h1_MC_Mult->Fill(primaries);
        }

				
        // GTPC Point information[points inside tha Active Region]
        if (checkPoints)
        {
            if (graphicalOutput)
                h1_Point_Mult->Fill(pointsPerEvent);
            double ll = 0, ee=0;
            for (Int_t h = 0; h < pointsPerEvent; h++)//HERE
            {
								if(point[h]->GetPDGCode()==-211&&point[h]->GetParentTrackID()==-1)
								{
								//cout<<"Name="<<point[h]->GetParticleName()<<"	ParentID="<<point[h]->GetParentTrackID()<<"	TrackStatus="<<point[h]->GetTrackStatus()<<"	Volume="<<point[h]->GetVolName()<<"	TrackStep="<<point[h]->GetTrackStep()<<"	X="<<point[h]->GetX()<<"	Z="<<point[h]->GetZ()<<endl;
								ll+=point[h]->GetTrackStep();
								ee+=point[h]->GetEnergyLoss() * 1000;
								} 
                if (textOutput)
                {
                    cout << "Point number " << h << endl;
                    point[h]->Print();
                }
                if (graphicalOutput)
                {
                    h1_Point_Time->Fill(point[h]->GetTime());
                    h1_Point_Length->Fill(point[h]->GetLength());
                    h1_Point_ELoss->Fill(point[h]->GetEnergyLoss() * 1000);
                    h1_Point_TrackStatus->Fill(point[h]->GetTrackStatus());
                    h1_Point_PDG->Fill(point[h]->GetPDGCode());
                    h1_Point_Charge->Fill(point[h]->GetCharge());
                    h1_Point_Mass->Fill(point[h]->GetMass());
                    h1_Point_Kine->Fill(point[h]->GetKineticEnergy() * 1000);
                    h1_Point_trackStep->Fill(point[h]->GetTrackStep());
                    //h2_Point_XZ->Fill(point[h]->GetX(), point[h]->GetZ());
                    //h2_Point_YZ->Fill(point[h]->GetY(), point[h]->GetZ()); 
                    if(point[h]->GetPDGCode()==-211)h2_Point_XZ->Fill(point[h]->GetZ(),point[h]->GetX());//1000060120
                    if(point[h]->GetPDGCode()==-211)h2_Point_YZ->Fill(point[h]->GetZ(), point[h]->GetY());                    
                    h2_Point_PxPz->Fill(point[h]->GetPx(), point[h]->GetPz());
                    h2_Point_PyPz->Fill(point[h]->GetPy(), point[h]->GetPz());
                }
            }
            if(ll>2.&&zz>162.5)
            {
            h1_Pions_TrackLength->Fill(ll);
            h1_Pions_ELoss->Fill(ee);
            }
        }

        // User defined additions

        if (pointsPerEvent)
            delete[] point;
        if (MCtracksPerEvent)
            delete[] track;
    }

    if (graphicalOutput)
    {
        TCanvas* c1 = new TCanvas("MCTrack", "MCTrack", 0, 0, 500, 700);
        c1->SetFillColor(0);
        c1->SetFrameFillColor(0);
        TCanvas* c2 = new TCanvas("Points", "Points", 20, 20, 620, 720);
        c2->SetFillColor(0);
        c2->SetFrameFillColor(0);
        TCanvas* c3 = new TCanvas("Points Position", "Points Position", 40, 40, 540, 740);
        c3->SetFillColor(0);
        c3->SetFrameFillColor(0);
        TCanvas* c4 = new TCanvas("Pions properties", "Pions properties", 40, 40, 540, 740);
        c4->SetFillColor(0);
        c4->SetFrameFillColor(0);

        // MC TRACK CANVAS
        c1->cd();
        c1->Divide(2, 3);
        c1->cd(1);
        c1->cd(1)->SetLogy();
        h1_MC_Mult->Draw();
        c1->cd(2);
        c1->cd(2)->SetLogy();
        h1_MC_MultSec->Draw();
        c1->cd(3);
        c1->cd(3)->SetLogy();
        h1_MC_PDGSec->SetLineColor(kRed);
        h1_MC_PDGSec->Draw();
        h1_MC_PDG->Draw("SAME");
        TLatex Tl;
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(200, 10, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(1400, 10, "Secondaries");
        c1->cd(4);
        c1->cd(4)->SetLogy();
        h1_MC_EneSec->SetLineColor(kRed);
        h1_MC_EneSec->Draw();
        h1_MC_Ene->Draw("SAME");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(5 * maxE, 10, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(20 * maxE, 10, "Secondaries");
        c1->cd(5);
        c1->cd(5)->SetLogy();
        h1_MC_ThetaSec->SetLineColor(kRed);
        h1_MC_ThetaSec->Draw();
        h1_MC_Theta->Draw("SAME");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(0.2, 2, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(1.8, 2, "Secondaries");
        c1->cd(6);
        c1->cd(6)->SetLogy();
        h1_MC_PhiSec->SetLineColor(kRed);
        h1_MC_PhiSec->Draw();
        h1_MC_Phi->Draw("SAME");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(-2, 2, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(0.8, 2, "Secondaries");

        // POINT CANVAS
        c2->cd();
        c2->Divide(3, 3);
        c2->cd(1);
        c2->cd(1)->SetLogy();
        h1_Point_Mult->Draw();
        c2->cd(2);
        c2->cd(2)->SetLogy();
        h1_Point_Time->Draw();
        c2->cd(3);
        c2->cd(3)->SetLogy();
        h1_Point_Length->Draw();
        c2->cd(4);
        c2->cd(4)->SetLogy();
        h1_Point_ELoss->Draw();
        c2->cd(5);
        c2->cd(5)->SetLogy();
        h1_Point_trackStep->Draw();
        c2->cd(6);
        c2->cd(6)->SetLogy();
        h1_Point_PDG->Draw();
        c2->cd(7);
        c2->cd(7)->SetLogy();
        h1_Point_Charge->Draw();
        c2->cd(8);
        c2->cd(8)->SetLogy();
        h1_Point_Mass->Draw();
        c2->cd(9);
        c2->cd(9)->SetLogy();
        h1_Point_Kine->Draw();
        // c2->cd(1); c2->cd(1)->SetLogy();h1_Point_trackStep->Draw();

        // POINT CANVAS POSITION & MOMENTUM
        c3->cd();
        c3->Divide(2, 2);
        c3->cd(1);
        h2_Point_XZ->Draw("col");
        c3->cd(2);
        h2_Point_YZ->Draw("col");
        c3->cd(3);
        h2_Point_PxPz->Draw("col");
        c3->cd(4);
        h2_Point_PyPz->Draw("col");
        //Pions properties
        c4->cd();
        c4->Divide(1, 2);
        c4->cd(1);
        h1_Pions_TrackLength->Draw();
        c4->cd(2);
        h1_Pions_ELoss->Draw();
        
        cout << "\n" << endl;
				// -----   Finish   -------------------------------------------------------
				timer.Stop();
				Double_t rtime = timer.RealTime();
				Double_t ctime = timer.CpuTime();
				cout << endl << endl;
				cout << "Macro finished succesfully." << endl;
				cout << "Real time " << rtime/60 << " min, CPU time " << ctime/60 << "min" << endl << endl;
        // OUTPUT FILE
        if (GEOTAG.CompareTo("FullBeamIn")==0)
        {
        c1->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps(");
        c2->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps");
        c3->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps");
        c4->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps)");
        }
        else
        {
        c1->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps(");
        c2->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps");
        c3->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps");
        c4->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps)");
        }
    }
}
