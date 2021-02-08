//  -------------------------------------------------------------------------
//
//   -----  Macro to map the distortion and resolution with the langevin macro
//         Author: Hector Alvarez <hector.alvarez@usc.es>
//         Comments:
//
//  -------------------------------------------------------------------------
//
//   Usage:
//      > root -l map_lang.C
//
//     BUT FIRST, select in the //SETTINGS section the simulation features
//	(the macro will plot and text information as a function of these settings)
//  -------------------------------------------------------------------------
void map_lang()
{

    Int_t FIT = 0;
    Int_t STAT = 1;
    Int_t SHOW = 1;
    Int_t eventShown = 1;

    ofstream myfile;
    myfile.open("results_fit_0.txt");
    ofstream myfile2;
    myfile2.open("results_stat_0.txt");

    char title1[250];

    // SETTINGS
    // langevin results
    sprintf(title1, "%s", "lang_TOMC_100_borra2.root"); // INPUT FILE 1  TODO ???????
    TFile* file1 = TFile::Open(title1);

    gROOT->SetStyle("Default");
    gStyle->SetOptTitle(1);
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1111);

    // SET THIS VALUES AS IT WAS IN THE R3BGTPCProjector code
    Double_t fHalfSizeTPC_X = 40;     // 50cm in X (row)
    Double_t fHalfSizeTPC_Y = 20;     // 20cm in Y (time)
    Double_t fHalfSizeTPC_Z = 100;    // 100cm in Z (column)
    Double_t fSizeOfVirtualPad = 100; // 1: pads of 1cm^2 , 10: pads of 1mm^2

    // END OF SETTINGS

    // HISTOS
    UInt_t histoBins_x = 50;
    UInt_t histoBins_z = 50;
    char tit[250];
    char name[250];
    char namef2[250];

    TH2I** htrackInPads = new TH2I*[585];

    for (Int_t gridPoint_z = 0; gridPoint_z < 39; gridPoint_z++)
    { // 39 points from 5 to 195 cm (200cm long TPC)
        for (Int_t gridPoint_x = 0; gridPoint_x < 15; gridPoint_x++)
        { // 15 points from 5 to 75 cm (80cm wide TPC)
            sprintf(tit, "hist_%i", gridPoint_z * 15 + gridPoint_x);
            sprintf(name, "(X,Z)=(%i cm,%i cm)", gridPoint_x * 5 + 5, gridPoint_z * 5 + 5);
            htrackInPads[gridPoint_z * 15 + gridPoint_x] = new TH2I(tit,
                                                                    name,
                                                                    histoBins_x,
                                                                    gridPoint_x * 500 + 250,
                                                                    gridPoint_x * 500 + 750,
                                                                    histoBins_z,
                                                                    gridPoint_z * 500 + 250,
                                                                    gridPoint_z * 500 + 750); // in [pad number]
            htrackInPads[gridPoint_z * 15 + gridPoint_x]->SetYTitle("Z [0.1mm]");
            htrackInPads[gridPoint_z * 15 + gridPoint_x]->SetXTitle("X [0.1mm]");
        }
    }
    // END OF HISTOS

    cout << "Values taken for the visualization" << endl
         << "bins in X: " << histoBins_x << endl
         << "bins in Z: " << histoBins_z << endl
         << "X size (half-length of box): " << fHalfSizeTPC_X << endl
         << "Y size (half-length of box (drift)): " << fHalfSizeTPC_Y << endl
         << "Z size (half-length of box): " << fHalfSizeTPC_Z << endl;

    TTree* TEvt_lan = (TTree*)file1->Get("evt");

    // GTPCPoints in langevin
    TClonesArray* gtpcProjPointCA_lan;
    R3BGTPCProjPoint** ppoint_lan;
    gtpcProjPointCA_lan = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCPoint_lan = TEvt_lan->GetBranch("GTPCProjPoint");
    branchGTPCPoint_lan->SetAddress(&gtpcProjPointCA_lan);

    Long64_t nevents_lan = TEvt_lan->GetEntries();
    cout << "INFO:  The number of events is " << nevents_lan << endl;
    Int_t ppointsPerEvent_lan = 0;
    Int_t x_pad, z_pad;

    // for(Int_t i=0;i<nevents_lan;i++){
    //  printf("Event:%i\n",i);

    gtpcProjPointCA_lan->Clear();
    TEvt_lan->GetEvent(eventShown);
    ppointsPerEvent_lan = gtpcProjPointCA_lan->GetEntries();

    cout << "INFO:  The number of ppoints_lan in this event is " << ppointsPerEvent_lan << endl;

    if (ppointsPerEvent_lan > 0)
    {
        ppoint_lan = new R3BGTPCProjPoint*[ppointsPerEvent_lan];
        for (Int_t j = 0; j < ppointsPerEvent_lan; j++)
        {
            // ppoint_lan[j] = new R3BGTPCProjPoint;
            ppoint_lan[j] = (R3BGTPCProjPoint*)gtpcProjPointCA_lan->At(j);
        }
    }

    // FIRST, filling the pad elements from projection and langevin approaches
    if (ppointsPerEvent_lan > 0)
    {
        for (Int_t j = 0; j < ppointsPerEvent_lan; j++)
        {
            x_pad = ppoint_lan[j]->GetVirtualPadID() % (Int_t)(2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
            z_pad = (ppoint_lan[j]->GetVirtualPadID() - x_pad) / (2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
            for (Int_t ff = 0; ff < 585; ff++)
            {
                htrackInPads[ff]->Fill(x_pad, z_pad, ppoint_lan[j]->GetCharge());
            }
        }
    }

    if (ppointsPerEvent_lan)
        delete[] ppoint_lan;

    //} NO ITERATION

    if (FIT)
    {
        for (Int_t gridPoint_z = 0; gridPoint_z < 39; gridPoint_z++)
        { // 39 points from 5 to 195 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 5 to 75 cm (80cm wide TPC)
                sprintf(namef2, "xigaus_%i", gridPoint_z * 15 + gridPoint_x);
                TF2* f2 = new TF2(namef2,
                                  "xygaus",
                                  gridPoint_x * 500 + 350,
                                  gridPoint_x * 500 + 650,
                                  gridPoint_z * 500 + 350,
                                  gridPoint_z * 500 + 650);
                f2->SetParameters(25, gridPoint_x * 500 + 500, 30, gridPoint_z * 500 + 500, 30);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Fit(namef2);
                f2->SetParameters(25, gridPoint_x * 500 + 500, 30, gridPoint_z * 500 + 500, 30);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Fit(namef2);
                myfile << "X=" << gridPoint_x * 500 + 500 << " Y=" << gridPoint_z * 500 + 500 << " "
                       << f2->GetParameter(0) << " " << f2->GetParameter(1) << f2->GetParameter(2) << " "
                       << f2->GetParameter(3) << f2->GetParameter(4) << "\n";
            }
        }
    }

    if (STAT)
    {
        for (Int_t gridPoint_z = 0; gridPoint_z < 39; gridPoint_z++)
        { // 39 points from 5 to 195 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 5 to 75 cm (80cm wide TPC)
                myfile2 << "X=" << gridPoint_x * 500 + 500 << " Z=" << gridPoint_z * 500 + 500 << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMean(1) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMeanError(1) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMean(1) - (gridPoint_x * 500 + 500) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMeanError(1) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMean(2) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMeanError(2) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMean(2) - (gridPoint_z * 500 + 500) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetMeanError(2) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetStdDev(1) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetStdDevError(1) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetStdDev(2) << "+-"
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetStdDevError(2) << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetCorrelationFactor() << " "
                        << htrackInPads[gridPoint_z * 15 + gridPoint_x]->GetCovariance() << "\n";
            }
        }
    }

    if (SHOW)
    {
        TCanvas* c11 = new TCanvas("c11", "Z=[5,40]cm, X=[5,25]cm", 0, 0, 400, 900);
        c11->Divide(5, 8);
        for (Int_t gridPoint_z = 0; gridPoint_z < 8; gridPoint_z++)
        { // 8 points from 5 to 40 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 5; gridPoint_x++)
            { // 5 points from 5 to 25 cm (80cm wide TPC)
                c11->cd(gridPoint_z * 5 + gridPoint_x + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c12 = new TCanvas("c12", "Z=[5,40]cm, X=[30,50]cm", 0, 0, 400, 900);
        c12->Divide(5, 8);
        for (Int_t gridPoint_z = 0; gridPoint_z < 8; gridPoint_z++)
        { // 8 points from 5 to 40 cm (200cm long TPC)
            for (Int_t gridPoint_x = 5; gridPoint_x < 10; gridPoint_x++)
            { // 15 points from 30 to 50 cm (80cm wide TPC)
                c12->cd(gridPoint_z * 5 + (gridPoint_x - 5) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c13 = new TCanvas("c13", "Z=[5,40]cm, X=[50,75]cm", 0, 0, 400, 900);
        c13->Divide(5, 8);
        for (Int_t gridPoint_z = 0; gridPoint_z < 8; gridPoint_z++)
        { // 8 points from 5 to 40 cm (200cm long TPC)
            for (Int_t gridPoint_x = 10; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 55 to 75 cm (80cm wide TPC)
                c13->cd(gridPoint_z * 5 + (gridPoint_x - 10) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }

        TCanvas* c21 = new TCanvas("c21", "Z=[45,80]cm, X=[5,25]cm", 0, 0, 400, 900);
        c21->Divide(5, 8);
        for (Int_t gridPoint_z = 8; gridPoint_z < 16; gridPoint_z++)
        { // 8 points from 45 to 80 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 5; gridPoint_x++)
            { // 5 points from 5 to 25 cm (80cm wide TPC)
                c21->cd((gridPoint_z - 8) * 5 + gridPoint_x + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c22 = new TCanvas("c22", "Z=[45,80]cm, X=[30,50]cm", 0, 0, 400, 900);
        c22->Divide(5, 8);
        for (Int_t gridPoint_z = 8; gridPoint_z < 16; gridPoint_z++)
        { // 8 points from 45 to 80 cm (200cm long TPC)
            for (Int_t gridPoint_x = 5; gridPoint_x < 10; gridPoint_x++)
            { // 15 points from 30 to 50 cm (80cm wide TPC)
                c22->cd((gridPoint_z - 8) * 5 + (gridPoint_x - 5) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c23 = new TCanvas("c23", "Z=[45,80]cm, X=[50,75]cm", 0, 0, 400, 900);
        c23->Divide(5, 8);
        for (Int_t gridPoint_z = 8; gridPoint_z < 16; gridPoint_z++)
        { // 8 points from 45 to 80 cm (200cm long TPC)
            for (Int_t gridPoint_x = 10; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 55 to 75 cm (80cm wide TPC)
                c23->cd((gridPoint_z - 8) * 5 + (gridPoint_x - 10) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }

        TCanvas* c31 = new TCanvas("c31", "Z=[85,120]cm, X=[5,25]cm", 0, 0, 400, 900);
        c31->Divide(5, 8);
        for (Int_t gridPoint_z = 16; gridPoint_z < 24; gridPoint_z++)
        { // 8 points from 85 to 120 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 5; gridPoint_x++)
            { // 5 points from 5 to 25 cm (80cm wide TPC)
                c31->cd((gridPoint_z - 16) * 5 + gridPoint_x + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c32 = new TCanvas("c32", "Z=[85,120]cm, X=[30,50]cm", 0, 0, 400, 900);
        c32->Divide(5, 8);
        for (Int_t gridPoint_z = 16; gridPoint_z < 24; gridPoint_z++)
        { // 8 points from 85 to 120 cm (200cm long TPC)
            for (Int_t gridPoint_x = 5; gridPoint_x < 10; gridPoint_x++)
            { // 15 points from 30 to 50 cm (80cm wide TPC)
                c32->cd((gridPoint_z - 16) * 5 + (gridPoint_x - 5) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c33 = new TCanvas("c33", "Z=[85,120]cm, X=[50,75]cm", 0, 0, 400, 900);
        c33->Divide(5, 8);
        for (Int_t gridPoint_z = 16; gridPoint_z < 24; gridPoint_z++)
        { // 8 points from 85 to 120 cm (200cm long TPC)
            for (Int_t gridPoint_x = 10; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 55 to 75 cm (80cm wide TPC)
                c33->cd((gridPoint_z - 16) * 5 + (gridPoint_x - 10) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }

        TCanvas* c41 = new TCanvas("c41", "Z=[125,160]cm, X=[5,25]cm", 0, 0, 400, 900);
        c41->Divide(5, 8);
        for (Int_t gridPoint_z = 24; gridPoint_z < 32; gridPoint_z++)
        { // 8 points from 125 to 160 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 5; gridPoint_x++)
            { // 5 points from 5 to 25 cm (80cm wide TPC)
                c41->cd((gridPoint_z - 24) * 5 + gridPoint_x + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c42 = new TCanvas("c42", "Z=[125,160]cm, X=[30,50]cm", 0, 0, 400, 900);
        c42->Divide(5, 8);
        for (Int_t gridPoint_z = 24; gridPoint_z < 32; gridPoint_z++)
        { // 8 points from 125 to 160 cm (200cm long TPC)
            for (Int_t gridPoint_x = 5; gridPoint_x < 10; gridPoint_x++)
            { // 15 points from 30 to 50 cm (80cm wide TPC)
                c42->cd((gridPoint_z - 24) * 5 + (gridPoint_x - 5) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c43 = new TCanvas("c43", "Z=[125,160]cm, X=[50,75]cm", 0, 0, 400, 900);
        c43->Divide(5, 8);
        for (Int_t gridPoint_z = 24; gridPoint_z < 32; gridPoint_z++)
        { // 8 points from 125 to 160 cm (200cm long TPC)
            for (Int_t gridPoint_x = 10; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 55 to 75 cm (80cm wide TPC)
                c43->cd((gridPoint_z - 24) * 5 + (gridPoint_x - 10) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }

        TCanvas* c51 = new TCanvas("c51", "Z=[165,195]cm, X=[5,25]cm", 0, 0, 400, 900);
        c51->Divide(5, 7);
        for (Int_t gridPoint_z = 32; gridPoint_z < 39; gridPoint_z++)
        { // 8 points from 165 to 195 cm (200cm long TPC)
            for (Int_t gridPoint_x = 0; gridPoint_x < 5; gridPoint_x++)
            { // 5 points from 5 to 25 cm (80cm wide TPC)
                c51->cd((gridPoint_z - 32) * 5 + gridPoint_x + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c52 = new TCanvas("c52", "Z=[165,195]cm, X=[30,50]cm", 0, 0, 400, 900);
        c52->Divide(5, 7);
        for (Int_t gridPoint_z = 32; gridPoint_z < 39; gridPoint_z++)
        { // 8 points from 165 to 195 cm (200cm long TPC)
            for (Int_t gridPoint_x = 5; gridPoint_x < 10; gridPoint_x++)
            { // 15 points from 30 to 50 cm (80cm wide TPC)
                c52->cd((gridPoint_z - 32) * 5 + (gridPoint_x - 5) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
        TCanvas* c53 = new TCanvas("c53", "Z=[165,195]cm, X=[50,75]cm", 0, 0, 400, 900);
        c53->Divide(5, 7);
        for (Int_t gridPoint_z = 32; gridPoint_z < 39; gridPoint_z++)
        { // 8 points from 165 to 195 cm (200cm long TPC)
            for (Int_t gridPoint_x = 10; gridPoint_x < 15; gridPoint_x++)
            { // 15 points from 55 to 75 cm (80cm wide TPC)
                c53->cd((gridPoint_z - 32) * 5 + (gridPoint_x - 10) + 1);
                htrackInPads[gridPoint_z * 15 + gridPoint_x]->Draw("ZCOL");
            }
        }
    }

    myfile.close();
    myfile2.close();
}
