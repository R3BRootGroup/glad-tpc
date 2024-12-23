//  -------------------------------------------------------------------------
//
//   -----  Macro to check and analize langevin macro results
//         Author: Hector Alvarez <hector.alvarez@usc.es>
//         Comments:
//
//  -------------------------------------------------------------------------
//
//   Usage:
//      > root -l ana_lang.C
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
void ana_lang(TString GEOTAG = "Prototype")
{

    // Output file1
    TString title1;
    // Output file2
    TString title2;

    // Input and outup file according to the GEOTAG
    TString GTPCGeoParamsFile;
    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./Prototype/lang.root";
        title2 = "./Prototype/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";
    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./FullBeamOut/lang.root";
        title2 = "./FullBeamOut/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAFullBeamOut_FileSetup.par";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        title1 = "./FullBeamIn/lang.root";
        title2 = "./FullBeamIn/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAFullBeamIn_FileSetup.par";
    }

    TFile* file1 = TFile::Open(title1); // langevin results
    TFile* file2 = TFile::Open(title2); // simple projection results

    gROOT->SetStyle("Default");
    gStyle->SetOptTitle(1);
    gStyle->SetOptStat(111111);
    gStyle->SetOptFit(0);

    // RTDB
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    R3BGTPCGeoPar* geoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!geoPar)
    {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }

    Double_t fHalfSizeTPC_X = geoPar->GetActiveRegionx() / 2; // 50cm in X (row)
    Double_t fHalfSizeTPC_Y = geoPar->GetActiveRegiony() / 2; // 20cm in Y (time)
    Double_t fHalfSizeTPC_Z = geoPar->GetActiveRegionz() / 2; // 100cm in Z (column)
    Double_t fSizeOfVirtualPad = segeoPartup->GetPadSize();   // 1: pads of 1cm^2 , 10: pads of 1mm^2

    // END OF SETTINGS

    // HISTOS
    UInt_t histoBins = 2 * fHalfSizeTPC_X * fSizeOfVirtualPad;
    UInt_t histoBins2 = 2 * fHalfSizeTPC_Z * fSizeOfVirtualPad;

    TH2D* htrackInPads_x = new TH2D("htrackInPads_x",
                                    "Diff in X in the XZ Pads Plane",
                                    histoBins,
                                    0,
                                    2 * fHalfSizeTPC_X * fSizeOfVirtualPad,
                                    histoBins2,
                                    0,
                                    2 * fHalfSizeTPC_Z * fSizeOfVirtualPad); // in [pad number]
    htrackInPads_x->SetYTitle("Z [pad number]");
    htrackInPads_x->SetXTitle("X [pad number]");
    TH2D* htrackInPads_z = new TH2D("htrackInPads_z",
                                    "Diff in Z in the XZ Pads Plane",
                                    histoBins,
                                    0,
                                    2 * fHalfSizeTPC_X * fSizeOfVirtualPad,
                                    histoBins2,
                                    0,
                                    2 * fHalfSizeTPC_Z * fSizeOfVirtualPad); // in [pad number]
    htrackInPads_z->SetYTitle("Z [pad number]");
    htrackInPads_z->SetXTitle("X [pad number]");

    // END OF HISTOS

    cout << "Values taken for the visualization" << endl
         << "bins in X: " << histoBins << endl
         << "bins in Z: " << histoBins2 << endl
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

    TTree* TEvt = (TTree*)file2->Get("evt");

    // GTPCPoints in simple projection
    TClonesArray* gtpcProjPointCA;
    R3BGTPCProjPoint** ppoint;
    gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCPoint = TEvt->GetBranch("GTPCProjPoint");
    branchGTPCPoint->SetAddress(&gtpcProjPointCA);

    Long64_t nevents = TEvt->GetEntries();
    Long64_t nevents_lan = TEvt_lan->GetEntries();
    if (nevents != nevents_lan)
        cout << "ERROR: different number of entries" << endl;
    cout << "INFO:  The number of events is " << nevents << endl;
    Int_t ppointsPerEvent = 0;
    Int_t ppointsPerEvent_lan = 0;
    Double_t *xPad, *zPad, *tPad, *chargePad;
    Double_t *xPad_lan, *zPad_lan, *tPad_lan, *chargePad_lan;
    Double_t x_mean[100];
    Double_t x_mean_lan[100];
    Double_t z_mean[50];
    Double_t z_mean_lan[50];
    Double_t charge_x[100];
    Double_t charge_x_lan[100];
    Double_t charge_z[50];
    Double_t charge_z_lan[50];
    Int_t times_x[50][100];
    Int_t times_z[50][100];
    Double_t differs_x = 0;
    Double_t differs_z = 0;
    Double_t oldmean = 0;

    for (Int_t ccz = 0; ccz < 100; ccz++)
    {
        for (Int_t ccx = 0; ccx < 50; ccx++)
        {
            times_x[ccx][ccz] = 0;
            times_z[ccx][ccz] = 0;
        }
    }
    for (Int_t ccz = 0; ccz < 101; ccz++)
    {
        for (Int_t ccx = 0; ccx < 51; ccx++)
        {
            htrackInPads_x->SetBinContent(ccx, ccz, 0.);
            htrackInPads_z->SetBinContent(ccx, ccz, 0.);
        }
    }

    for (Int_t i = 0; i < nevents; i++)
    {
        double percentage = i / (double)(nevents * 1.);
        loadfunction(percentage);

        gtpcProjPointCA->Clear();
        gtpcProjPointCA_lan->Clear();

        TEvt->GetEvent(i);
        TEvt_lan->GetEvent(i);

        ppointsPerEvent = gtpcProjPointCA->GetEntries();
        ppointsPerEvent_lan = gtpcProjPointCA_lan->GetEntries();

        /*cout << endl <<"INFO:  The number of ppoints in this event is "
             << ppointsPerEvent << endl;
        cout << "INFO:  The number of ppoints_lan in this event is "
             << ppointsPerEvent_lan << endl;
        */
        if (ppointsPerEvent > 0)
        {
            ppoint = new R3BGTPCProjPoint*[ppointsPerEvent];
            for (Int_t j = 0; j < ppointsPerEvent; j++)
            {
                // ppoint[j] = new R3BGTPCProjPoint;
                ppoint[j] = (R3BGTPCProjPoint*)gtpcProjPointCA->At(j);
            }
        }
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
        if (ppointsPerEvent > 0 && ppointsPerEvent_lan > 0)
        {
            xPad = new Double_t[ppointsPerEvent];
            zPad = new Double_t[ppointsPerEvent];
            tPad = new Double_t[ppointsPerEvent];
            chargePad = new Double_t[ppointsPerEvent];
            for (Int_t h = 0; h < ppointsPerEvent; h++)
            {
                xPad[h] = ppoint[h]->GetVirtualPadID() % (Int_t)(2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                zPad[h] = (ppoint[h]->GetVirtualPadID() - xPad[h]) / (2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                tPad[h] = ((TH1S*)(ppoint[h]->GetTimeDistribution()))->GetMean();
                chargePad[h] = ppoint[h]->GetCharge();
            }
            xPad_lan = new Double_t[ppointsPerEvent_lan];
            zPad_lan = new Double_t[ppointsPerEvent_lan];
            tPad_lan = new Double_t[ppointsPerEvent_lan];
            chargePad_lan = new Double_t[ppointsPerEvent_lan];
            for (Int_t h = 0; h < ppointsPerEvent_lan; h++)
            {
                xPad_lan[h] = ppoint_lan[h]->GetVirtualPadID() % (Int_t)(2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                zPad_lan[h] =
                    (ppoint_lan[h]->GetVirtualPadID() - xPad_lan[h]) / (2 * fHalfSizeTPC_X * fSizeOfVirtualPad);
                tPad_lan[h] = ((TH1S*)(ppoint_lan[h]->GetTimeDistribution()))->GetMean();
                chargePad_lan[h] = ppoint_lan[h]->GetCharge();
            }
            // SECOND, calculate the mean for each track in x[z] and z[x], weighted by charge
            for (Int_t ccz = 0; ccz < 100; ccz++)
            {
                x_mean[ccz] = 0;
                x_mean_lan[ccz] = 0;
                charge_x[ccz] = 0;
                charge_x_lan[ccz] = 0;
            }
            for (Int_t ccx = 0; ccx < 50; ccx++)
            {
                z_mean[ccx] = 0;
                z_mean_lan[ccx] = 0;
                charge_z[ccx] = 0;
                charge_z_lan[ccx] = 0;
            }
            differs_x = differs_z = 0;

            for (Int_t h = 0; h < ppointsPerEvent; h++)
            {
                if (zPad[h] > 0. && zPad[h] < 100. && xPad[h] > 0. && xPad[h] < 50.)
                {
                    x_mean[(Int_t)zPad[h]] += chargePad[h] * xPad[h];
                    charge_x[(Int_t)zPad[h]] += chargePad[h];
                    z_mean[(Int_t)xPad[h]] += chargePad[h] * zPad[h];
                    charge_z[(Int_t)xPad[h]] += chargePad[h];
                }
            }
            for (Int_t ccz = 0; ccz < 100; ccz++)
            {
                if (charge_x[ccz] > 0)
                {
                    x_mean[ccz] = x_mean[ccz] / charge_x[ccz];
                    // cout << "x_mean["<<ccz<<"]=" << x_mean[ccz] << endl;
                }
            }
            for (Int_t ccx = 0; ccx < 50; ccx++)
            {
                if (charge_z[ccx] > 0)
                {
                    z_mean[ccx] = z_mean[ccx] / charge_z[ccx];
                    // cout << "z_mean["<<ccx<<"]=" << z_mean[ccx] << endl;
                }
            }
            for (Int_t h = 0; h < ppointsPerEvent_lan; h++)
            {
                if (zPad_lan[h] > 0. && zPad_lan[h] < 100. && xPad_lan[h] > 0. && xPad_lan[h] < 50.)
                {
                    x_mean_lan[(Int_t)zPad_lan[h]] += chargePad_lan[h] * xPad_lan[h];
                    charge_x_lan[(Int_t)zPad_lan[h]] += chargePad_lan[h];
                    z_mean_lan[(Int_t)xPad_lan[h]] += chargePad_lan[h] * zPad_lan[h];
                    charge_z_lan[(Int_t)xPad_lan[h]] += chargePad_lan[h];
                }
            }

            for (Int_t ccz = 0; ccz < 100; ccz++)
            {
                if (charge_x_lan[ccz] > 0)
                {
                    x_mean_lan[ccz] = x_mean_lan[ccz] / charge_x_lan[ccz];
                    // cout << "x_mean_lan["<<ccz<<"]=" << x_mean_lan[ccz] << endl;
                }
            }
            for (Int_t ccx = 0; ccx < 50; ccx++)
            {
                if (charge_z_lan[ccx] > 0)
                {
                    z_mean_lan[ccx] = z_mean_lan[ccx] / charge_z_lan[ccx];
                    // cout << "z_mean_lan["<<ccx<<"]=" << z_mean_lan[ccx] << endl;
                }
            }
            // THIRD; calculate the differences and introduce a recursive mean
            for (Int_t ccz = 0; ccz < 100; ccz++)
            {
                if (charge_x[ccz] > 0. && charge_x_lan[ccz] > 0.)
                {
                    if (x_mean_lan[ccz] > 0. && x_mean_lan[ccz] < 50.)
                    {
                        // cout << "x: " << (Int_t)x_mean_lan[ccz] << " "<< ccz << endl;
                        times_x[(Int_t)x_mean_lan[ccz]][ccz]++;
                        differs_x = x_mean[ccz] - x_mean_lan[ccz];
                        oldmean = htrackInPads_x->GetBinContent((Int_t)x_mean_lan[ccz], ccz);
                        // cout << "times_x[" <<(Int_t)x_mean_lan[ccz]<<"]"<<"["<<ccz<<"]="<<
                        // times_x[(Int_t)x_mean_lan[ccz]][ccz]
                        //	 << " differs_x=" << differs_x << " oldmean=" << oldmean
                        //	 << " Hist=" << oldmean + ((differs_x-oldmean) / (times_x[(Int_t)x_mean_lan[ccz]][ccz]))
                        //<<endl;
                        htrackInPads_x->SetBinContent(
                            x_mean_lan[ccz],
                            ccz,
                            oldmean + ((differs_x - oldmean) / (times_x[(Int_t)x_mean_lan[ccz]][ccz])));
                    }
                }
            }
            for (Int_t ccx = 0; ccx < 50; ccx++)
            {
                if (charge_z[ccx] > 0 && charge_z_lan[ccx] > 0)
                {
                    if (z_mean_lan[ccx] > 0. && z_mean_lan[ccx] < 100.)
                    {
                        // cout << "z: "<< ccx << (Int_t)z_mean_lan[ccx] << endl;
                        times_z[ccx][(Int_t)z_mean_lan[ccx]]++;
                        differs_z = z_mean[ccx] - z_mean_lan[ccx];
                        oldmean = htrackInPads_x->GetBinContent(ccx, (Int_t)z_mean_lan[ccx]);
                        htrackInPads_z->SetBinContent(
                            ccx,
                            z_mean_lan[ccx],
                            oldmean + ((differs_z - oldmean) / (times_z[ccx][(Int_t)z_mean_lan[ccx]])));
                    }
                }
            }
        }

        if (ppointsPerEvent)
            delete[] ppoint;
        if (ppointsPerEvent_lan)
            delete[] ppoint_lan;
        if (ppointsPerEvent && ppointsPerEvent_lan)
        {
            delete xPad;
            delete zPad;
            delete tPad;
            delete chargePad;
            delete xPad_lan;
            delete zPad_lan;
            delete tPad_lan;
            delete chargePad_lan;
        }
    }
    cout << "\n" << endl;
    TCanvas* c1 = new TCanvas("c1", "Diff in pads charge (XZ) plane", 0, 0, 600, 900);

    c1->SetFillColor(0);
    c1->Divide(2);
    c1->Draw();
    TVirtualPad* c1_1 = c1->cd(1);
    // c1_1->SetLogz();
    htrackInPads_x->Draw("ZCOL");

    TVirtualPad* c1_2 = c1->cd(2);
    // c1_2->SetLogz();
    htrackInPads_z->Draw("ZCOL");
}
