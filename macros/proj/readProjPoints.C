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
//Loading bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void loadfunction(double &percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"
#define cGREEN "\033[1;32m"
void readProjPoints(TString GEOTAG = "Prototype")
{

    char hname[255];

    // SETTINGS
    Bool_t graphicalOutput = kTRUE;
    Bool_t textOutput = kFALSE;
    Bool_t checkProjPoints = kTRUE;    // Defines Histogram for ProjPoints
    Bool_t visualizeVPadPlane = kTRUE; // Defines VpadPlane viewer

    // INPUT FILE
    TString title1;
    if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./Prototype/proj.root";
    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./FullBeamOut/proj.root";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./FullBeamIn/proj.root";
    }

		TFile* file1 = TFile::Open(title1);
    // END OF THE SETTING AREA

    gROOT->SetStyle("Default");
    gStyle->SetOptStat(1111111);
    gStyle->SetOptFit(1);

    // HISTOGRAMS DEFINITION
    TH1F *h1_ProjPoint_Mult, *h1_ProjPoint_Charge, *h1_ProjPoint_VirtualPadID;
    //TH1S** h1_ProjPoint_TimeExample;
    if (graphicalOutput && checkProjPoints)
    { // HISTOGRAMS DEFINITION FOR PROJPOINTS
        h1_ProjPoint_Mult = new TH1F("h1_ProjPoint_Mult", "ProjPoint Multiplicity", 400, 0, 400);
        h1_ProjPoint_Charge = new TH1F("h1_ProjPoint_Charge;# primary e-", "ProjPoint Charge", 1000, 0, 1000);
        h1_ProjPoint_VirtualPadID = new TH1F("h1_ProjPoint_VirtualPadID", "ProjPoint VirtualPadID", 13000, -10, 12990);
        // h1_ProjPoint_TimeExample = new TH1F("h1_ProjPoint_TimeExample","ProjPoint TimeExample",100,0,100);
    }
    // END OF HISTOGRMAS DEFINITION
    TTree* TEvt = (TTree*)file1->Get("evt");

    // GTPCProjPoints
    TClonesArray* gtpcProjPointCA;
    R3BGTPCProjPoint** ppoint;
    gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
    branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

    Long64_t nevents = TEvt->GetEntries();
    if (textOutput)
        cout << "INFO:  The number of events is " << nevents << endl;

    Int_t ppointsPerEvent = 0;
    Int_t numberOfTimeHistos = 0;

    for (Int_t i = 0; i < nevents; i++)
    {
        double percentage=i/(double)(nevents*1.);
        loadfunction(percentage);
        gtpcProjPointCA->Clear();

        TEvt->GetEvent(i);

        ppointsPerEvent = gtpcProjPointCA->GetEntries();
        if (textOutput)
        {
            cout << endl << "INFO:  The number of projPoints in this event is " << ppointsPerEvent << endl;
        }

        if (ppointsPerEvent > 0)
        {
            ppoint = new R3BGTPCProjPoint*[ppointsPerEvent];
            for (Int_t j = 0; j < ppointsPerEvent; j++)
            {
                ppoint[j] = new R3BGTPCProjPoint;
                ppoint[j] = (R3BGTPCProjPoint*)gtpcProjPointCA->At(j);
            }
        }
        // GTPC Point information
        if (checkProjPoints)
        {
            if (graphicalOutput)
                h1_ProjPoint_Mult->Fill(ppointsPerEvent);
            for (Int_t h = 0; h < ppointsPerEvent; h++)
            {
                if (textOutput)
                {
                    cout << "ProjPoint number " << h << endl;
                    ppoint[h]->Print();
                }
                if (graphicalOutput)
                {
                    h1_ProjPoint_Charge->Fill(ppoint[h]->GetCharge());
                    h1_ProjPoint_VirtualPadID->Fill(ppoint[h]->GetVirtualPadID());
                }
            }
        }

        if (ppointsPerEvent)
            delete[] ppoint;
    }
		cout<<"\n"<<endl;
    if (graphicalOutput)
    {
        TCanvas* c1 = new TCanvas("ProjPoints", "ProjPoints", 20, 20, 620, 720);
        c1->SetFillColor(0);
        c1->SetFrameFillColor(0);
        c1->cd();
        c1->Divide(2, 2);
        c1->cd(1);
        c1->cd(1)->SetLogy();
        h1_ProjPoint_Mult->Draw();
        c1->cd(2);
        c1->cd(2)->SetLogy();
        h1_ProjPoint_Charge->Draw();
        c1->cd(3);
        h1_ProjPoint_VirtualPadID->Draw();
        /*c1->cd(4);
        h1_ProjPoint_TimeExample[0]->GetXaxis()->SetRange(0, 50);
        h1_ProjPoint_TimeExample[0]->Draw();
        for (Int_t j = 1; j < 2; j++)
        {
            h1_ProjPoint_TimeExample[j]->SetLineColor(kRed);
            h1_ProjPoint_TimeExample[j]->Draw("SAME");
        }*/


        // OUTPUT FILE
        // ctext->Print("output.ps(");
        c1->Print("output_projpoints.ps");
        //c2->Print("output.ps)");
        // c3->Print("output.ps)");
    }
}
