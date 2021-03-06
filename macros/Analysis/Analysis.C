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

void Analysis(TString GEOTAG="FullBeamIn", TString version="v1")
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
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1);

    // HISTOGRAMS DEFINITION
    TH1F *h1_MC_Mult, *h1_MC_PDG, *h1_MC_Ene;
    TH1F *h1_MC_MultSec, *h1_MC_PDGSec, *h1_MC_EneSec, *h1_MC_Vertex_Z;
    TH1D *h1_Pions_TrackLength, *h1_Pions_ELoss;
    TH2F *h2_Point_XZ, *h2_Point_YZ, *h2_MC_Vertex_XZ, *h2_MC_Vertex_YZ;
    TH2D *h2_TrackLength_VertexZ;
    TH3F *h3_SpaceCharge;
    if (graphicalOutput && checkMCTracks)
    { // HISTOGRAMS DEFINITION FOR MCTRACKS
        h1_MC_Mult = new TH1F("h1_MC_Mult", "MCTrack Multiplicity (primaries)", 200, 0, 20);
        // if secondaries are included
        h1_MC_MultSec = new TH1F("h1_MC_MultSec", "MCTrack Multiplicity", 200, 0, 2000);
        h1_MC_PDG = new TH1F("h1_MC_PDG", "Primary PDG Code", 6600, -2300, 4300);
        h1_MC_PDGSec = new TH1F("h1_MC_PDGSec", "MCTrack PDG Code", 6600, -2300, 4300);
        h1_MC_Ene = new TH1F("h1_MC_Ene", "Primary Energy (MeV)", 200, 0, 6 * maxE);
        h1_MC_EneSec = new TH1F("h1_MC_EneSec", "MCTrack Energy (MeV)", 200, 0, 6 * maxE);
        h2_MC_Vertex_XZ = new TH2F("h2_MC_Vertex_XZ", "Primary vertex position XZ (cm)", 2000, 155, 355,200,-1,1);
        h2_MC_Vertex_YZ = new TH2F("h2_MC_Vertex_YZ", "Primary vertex position YZ (cm)", 2000, 155, 355,200,-1,1);
        h1_MC_Vertex_Z = new TH1F("h1_MC_Vertex_Z", "Primary vertex position Z (cm)", 145, 155, 300);
    }
    if (graphicalOutput && checkPoints)
    { // HISTOGRAMS DEFINITION FOR POINTS
        h2_Point_XZ = new TH2F("h2_Point_XZ", "Pions XZ plane- Active region", 1200, 175, 295, 400, -10, 30);
        h2_Point_YZ = new TH2F("h2_Point_YZ", "Pions YZ plane- Active region", 1200, 175, 295, 600, -30, 30);
        h1_Pions_TrackLength = new TH1D("h1_Pions_TrackLength", "Pions trackLength (cm)", 100, 0, 100);
        h1_Pions_ELoss = new TH1D("h1_Pions_ELoss", "Pions ELoss (MeV)", 200, 0, 0.2);
        h2_TrackLength_VertexZ= new TH2D("h2_TrackLength_VertexZ","Pions trackLength inside AR(cm) Vs Z vertex (cm)",145, 155, 300,100, 0, 100);
        h3_SpaceCharge = new TH3F("h3_SpaceCharge", "Space charge; Z[cm];X[cm];Y[cm]", 120, 175, 295, 200, -10, 30, 300, -30, 30);
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
    Int_t size=11;
		double eff_zz[size];			// Efficiency Vz Z vertex
		double a=163.5, b=a+10.;  // depends on the target position
    for (Int_t i = 0; i < nevents; i++)
    {
        double percentage = i / (double)(nevents * 1.);
        loadfunction(percentage);
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
                        h2_MC_Vertex_XZ->Fill(track[h]->GetStartZ(),track[h]->GetStartX());
                        h2_MC_Vertex_YZ->Fill(track[h]->GetStartZ(),track[h]->GetStartY());
                    }
                    if(track[h]->GetPdgCode()==-211) zz=track[h]->GetStartZ();
                }
                else
                {
                    momentum = track[h]->GetMomentumMass();
                    h1_MC_PDGSec->Fill(track[h]->GetPdgCode());
                    h1_MC_EneSec->Fill(track[h]->GetEnergy() * 1000 - track[h]->GetMass() * 1000);
                }
            }
            h1_MC_Mult->Fill(primaries);
        }

				
        // GTPC Point information[points inside tha Active Region]
        if (checkPoints)
        {
            double ll = 0, ee=0;
            for (Int_t h = 0; h < pointsPerEvent; h++)//HERE
            {
								if(point[h]->GetPDGCode()==-211&&point[h]->GetParentTrackID()==-1)
								{
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
                    if(point[h]->GetPDGCode()==-211)h2_Point_XZ->Fill(point[h]->GetZ(),point[h]->GetX());//1000060120 He 1000020030
                    if(point[h]->GetPDGCode()==-211)h2_Point_YZ->Fill(point[h]->GetZ(), point[h]->GetY()); 
                    if(point[h]->GetPDGCode()==-211)h3_SpaceCharge->Fill(point[h]->GetZ(),point[h]->GetX(),point[h]->GetY(),point[h]->GetEnergyLoss() * 1000);
                }

            }
            if(ll>10.&&zz>163.5)//170, 175
            {
		          h1_Pions_TrackLength->Fill(ll);
		          h1_Pions_ELoss->Fill(ee);
		          h1_MC_Vertex_Z->Fill(zz);
		         	h2_TrackLength_VertexZ->Fill(zz,ll);
            }
         for (unsigned int i = 0; i < size; i += 1)
         {
         	 if(ll>10.&&zz>a+10*i && zz<=b+10*i) eff_zz[i]++;
         }

        }

        // User defined additions

        if (pointsPerEvent)
            delete[] point;
        if (MCtracksPerEvent)
            delete[] track;
    }
    double zz_avg[size], ex[size], ey[size], sum_eff=0;
    for (unsigned int i = 0; i < size; i += 1)
    {
			zz_avg[i]=((a+10*i)+(b+10*i))*0.5;	//Z vertex average	
			eff_zz[i]=(eff_zz[i]/nevents)*100;	//Efficiency percentage
			sum_eff+=eff_zz[i];									//Total efficiency in %
			ex[i]=5;														//error size bin x axis
			ey[i]=0;														//error y axis
    }
    TGraphErrors *gr_efficiency_VertexZ=new TGraphErrors(size, zz_avg ,eff_zz,ex,ey);
    gr_efficiency_VertexZ->SetTitle("Pion Efficiency (%) Vs Z vertex (cm)");
    gr_efficiency_VertexZ->SetMarkerStyle(kFullCircle);
    if (graphicalOutput)
    {
        TCanvas* c1 = new TCanvas("MCTrack", "MCTrack", 0, 0, 500, 700);
        c1->SetFillColor(0);
        c1->SetFrameFillColor(0);
        TCanvas* c2 = new TCanvas("Points Position", "Points Position", 40, 40, 540, 740);
        c2->SetFillColor(0);
        c2->SetFrameFillColor(0);
        TCanvas* c3 = new TCanvas("Pions properties", "Pions properties", 40, 40, 540, 740);
        c3->SetFillColor(0);
        c3->SetFrameFillColor(0);
       /* TCanvas* c4 = new TCanvas("Space charge", "Space charge", 40, 40, 540, 740);
        c4->SetFillColor(0);
        c4->SetFrameFillColor(0);*/

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
        Tl.DrawLatex(-2000, 10, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(500, 10, "Secondaries");
        c1->cd(4);
        c1->cd(4)->SetLogy();
        h1_MC_EneSec->SetLineColor(kRed);
        h1_MC_EneSec->Draw();
        h1_MC_Ene->Draw("SAME");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(1 * maxE, 100, "Primaries");
        Tl.SetTextSize(0.06);
        Tl.SetTextColor(2);
        Tl.DrawLatex(3* maxE, 100, "Secondaries");
        c1->cd(5);
        h2_MC_Vertex_XZ->Draw("col");
        c1->cd(6);
        h2_MC_Vertex_YZ->Draw("col");

        // POINT CANVAS POSITION & MOMENTUM
        c2->cd();
        c2->Divide(1, 2);
        c2->cd(1);
        h2_Point_XZ->Draw("col");
        c2->cd(2);
        h2_Point_YZ->Draw("col");
        
        //Pions properties
        c3->cd();
        c3->Divide(2, 2);
        c3->cd(1);
        h1_Pions_TrackLength->Draw();
        c3->cd(2);
        h1_MC_Vertex_Z->Draw();
        c3->cd(3);
        h2_TrackLength_VertexZ->Draw("col");
        c3->cd(4);
				gr_efficiency_VertexZ->Draw("AP");
				Tl.SetTextSize(0.06);
        Tl.SetTextColor(1);
        Tl.DrawLatex(240, 12, Form("#varepsilon_{#pi^{-}}= %.2f",sum_eff));

				//SPACE CHARGE TODO review the space charge 
				//c4->cd();
				//h3_SpaceCharge->Draw();        
        cout << "\n" << endl;
				// -----   Finish   -------------------------------------------------------
				timer.Stop();
				Double_t rtime = timer.RealTime();
				Double_t ctime = timer.CpuTime();
				cout << endl << endl;
				cout << "Macro finished succesfully." << endl;
				cout << "Real time " << rtime << " sec, CPU time " << ctime<< "sec" << endl << endl;
        // OUTPUT FILE
        if (GEOTAG.CompareTo("FullBeamIn")==0)
        {
        c1->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps(");
        c2->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps");
        c3->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps)");
        //c4->Print("./"+GEOTAG+"/"+version+"/Analysis"+GEOTAG+"."+version+".ps)");
        }
        else
        {
        c1->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps(");
        c2->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps");
        c3->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps)");
        //c4->Print("./"+GEOTAG+"/Analysis"+GEOTAG+".ps)");
        }
    }
}
