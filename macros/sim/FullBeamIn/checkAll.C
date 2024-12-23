//  -------------------------------------------------------------------------
//
//   ----- General Macro for check and analize all results
//         Author: Hector Alvarez <hector.alvarez@usc.es>
//         Comments:
//			Checks the primary, Points and Hits characteristics.
//			User configurable for additional studies.
//  -------------------------------------------------------------------------
//
//   Usage:
//      > root -l checkAll.C
//
//     BUT FIRST, select in the //SETTINGS section the simulation features
//	(the macro will plot and text information as a function of these settings)
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

void checkAll()
{
    char title1[250];
    // SETTINGS
    Bool_t graphicalOutput = kTRUE;
    Bool_t textOutput = kFALSE;
    Bool_t DEBUG = kFALSE;

    Bool_t checkPoints = kTRUE; // Defines Histogram for Points

    sprintf(title1, "%s", "sim.root"); // INPUT FILE
    TFile* file1 = TFile::Open(title1);
    if (file1->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    Double_t maxE = 100; // max energy in plots

    gDebug = 0; //    Debug option
    // END OF THE SETTING AREA

    gROOT->SetStyle("Default");
    gStyle->SetOptStat(11);
    gStyle->SetOptFit(1);

    // HISTOGRAMS DEFINITION
    TH1F *h1_Point_Mult, *h1_Point_Time, *h1_Point_Length, *h1_Point_ELoss;
    TH1F *h1_Point_TrackStatus, *h1_Point_PDG, *h1_Point_Charge, *h1_Point_Mass;
    TH1F *h1_Point_Kine, *h1_Point_trackStep;
    TH2F *h2_Point_XZ, *h2_Point_YZ;
    TH2F *h2_Point_PxPz, *h2_Point_PyPz;

    if (graphicalOutput && checkPoints)
    { // HISTOGRAMS DEFINITION FOR POINTS
        h1_Point_Mult = new TH1F("h1_Point_Mult", "Point Multiplicity", 400, 0, 400);
        h1_Point_Time = new TH1F("h1_Point_Time", "Point Time", 4000, 0, 0.000001);
        h1_Point_Length = new TH1F("h1_Point_Length", "Point Length", 400, 0, 400);
        h1_Point_ELoss = new TH1F("h1_Point_ELoss", "Point ELoss (keV)", 500, 0, 20); // 0.01 cm
        h1_Point_TrackStatus = new TH1F("h1_Point_TrackStatus", "Point TrackStatus", 8, 0, 7);
        h1_Point_PDG = new TH1F("h1_Point_PDG", "Point PDG", 2300, 0, 2300);
        h1_Point_Charge = new TH1F("h1_Point_Charge", "Point Charge", 40, 0, 39);
        h1_Point_Mass = new TH1F("h1_Point_Mass", "Point Mass", 400, 0, 10);
        h1_Point_Kine = new TH1F("h1_Point_Kine", "Point KinE (MeV)", 400, 0, 6000);
        h1_Point_trackStep = new TH1F("h1_Point_trackStep", "Point trackStep (cm)", 300, 0, 5); // maxstep 0.1 cm
        h2_Point_XZ = new TH2F("h2_Point_XZ", "Points projection on XZ plane;Z[cm];X[cm]", 600, 240, 300, 500, -20, 40);
        h2_Point_YZ = new TH2F("h2_Point_YZ", "Point projection on YZ plane;Z[cm];Y[cm]", 300, 240, 300, 400, -30, 30);
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
        if (!DEBUG)
            loadfunction(percentage);
        double e_dep_gas = 0, track_length = 0, counter = 0;
        gtpcPointCA->Clear();
        primaries = 0;

        TEvt->GetEvent(i);
        pointsPerEvent = gtpcPointCA->GetEntries();
        if (textOutput)
        {
            cout << endl << "INFO:  The number of points in this event is " << pointsPerEvent << endl;
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
        // GTPC Point information
        if (checkPoints)
        {
            if (graphicalOutput)
                h1_Point_Mult->Fill(pointsPerEvent);
            for (Int_t h = 0; h < pointsPerEvent; h++)
            {
                if (textOutput)
                {
                    cout << "Point number " << h << endl;
                    point[h]->Print();
                }
                if (graphicalOutput)
                {
                    h1_Point_Time->Fill(point[h]->GetTime());
                    h1_Point_Length->Fill(point[h]->GetLength());
                    h1_Point_ELoss->Fill(point[h]->GetEnergyLoss() * 1000000); // keV
                    h1_Point_TrackStatus->Fill(point[h]->GetTrackStatus());
                    h1_Point_PDG->Fill(point[h]->GetPDGCode());
                    h1_Point_Charge->Fill(point[h]->GetCharge());
                    h1_Point_Mass->Fill(point[h]->GetMass());
                    h1_Point_Kine->Fill(point[h]->GetKineticEnergy() * 1000);
                    h1_Point_trackStep->Fill(point[h]->GetTrackStep());
                    h2_Point_XZ->Fill(point[h]->GetZ(), point[h]->GetX());
                    h2_Point_YZ->Fill(point[h]->GetZ(), point[h]->GetY());
                    h2_Point_PxPz->Fill(point[h]->GetPx(), point[h]->GetPz());
                    h2_Point_PyPz->Fill(point[h]->GetPy(), point[h]->GetPz());
                }
                e_dep_gas += point[h]->GetEnergyLoss() * 1e6;
                track_length += point[h]->GetTrackStep();
                counter++;
            }
        }

        // User defined additions

        if (pointsPerEvent)
            delete[] point;
        if (DEBUG)
        {
            cout << setprecision(4) << "E_dep=" << e_dep_gas << " keV,	track_length=" << track_length
                 << " cm,	Edep/length=" << e_dep_gas / track_length << " keV/cm,	#points=" << counter << endl;
        }
    }
    cout << "\n" << endl;
    if (graphicalOutput)
    {
        TCanvas* c2 = new TCanvas("Points", "Points", 20, 20, 620, 720);
        c2->SetFillColor(0);
        c2->SetFrameFillColor(0);
        TCanvas* c3 = new TCanvas("Points Position", "Points Position", 40, 40, 540, 740);
        c3->SetFillColor(0);
        c3->SetFrameFillColor(0);
        TCanvas* c4 = new TCanvas("Transport model", "Transport model", 40, 40, 540, 740);
        c4->SetFillColor(0);
        c4->SetFrameFillColor(0);

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
        h2_Point_XZ->Draw("COLZ");
        c3->cd(2);
        h2_Point_YZ->Draw("COLZ");
        c3->cd(3);
        h2_Point_PxPz->Draw("COLZ");
        c3->cd(4);
        h2_Point_PyPz->Draw("COLZ");

        c4->cd();
        c4->Divide(2, 2);
        c4->cd(1);
        h2_Point_XZ->Draw("COLZ");
        c4->cd(2);
        c4->cd(2)->SetLogy();
        h1_Point_ELoss->Draw();
        c4->cd(3);
        c4->cd(3)->SetLogy();
        h1_Point_trackStep->Draw();
        c4->cd(4);

        // OUTPUT FILE
        // ctext->Print("output.ps(");
        c2->Print("output.ps(");
        c4->Print("output.ps");
        c3->Print("output.ps)");
    }
    cout << "OK" << endl;
}
