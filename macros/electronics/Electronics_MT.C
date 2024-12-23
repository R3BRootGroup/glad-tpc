/*
Author: Simone Velardita
Date:		09/02/2021
Macro for the AGET electronics readout. All the times should be in ns
*/
// ROOT library
#include "TClonesArray.h"
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TMath.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>
// GTPC library
#include "../../gtpcdata/R3BGTPCProjPoint.h"
// C++ Library
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <math.h>
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <string>
#include <vector>
using namespace std;

// Loading bar
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
const double e_equ = 1.60217e-19; // electron charge to coulombs
// definition at the end of the code
void loadfunction(double& percentage);
double conv_fit(double x[], double p[]);
double polya(double x[], double p[]);
double conv(double x[], double p[]);

int main(int argc, char** argv)
{
    const char* inputSimFile = "../../proj/Prototype/proj.root";
    gSystem->Load("libR3BGTPC.so");
    // Timer
    TStopwatch timer;
    timer.Start();

    Bool_t Debug = kFALSE;
    // SETUP
    string geoTag = "Prototype";
    TString GTPCGeoParamsFile;
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";
    GTPCGeoParamsFile2 = geoPath + "/glad-tpc/params/Electronic_FileSetup.par";
    GTPCGeoParamsFile.ReplaceAll("//", "/");
    GTPCGeoParamsFile2.ReplaceAll("//", "/");

    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    R3BGTPCGeoPar* geoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!geoPar)
    {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }
    R3BGTPCGasPar* gasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!gasPar)
    {
        cout << "No R3BGTPCGasPar can be loaded from the rtdb";
        return;
    }
    R3BGTPCElecPar* elecPar = (R3BGTPCElecPar*)rtdb->getContainer("GTPCElecPar");
    if (!elecPar)
    {
        cout << "No R3BGTPCElecPar can be loaded from the rtdb";
        return;
    }

    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    parIo1->open(GTPCGeoParamsFile, "in");
    parIo2->open(GTPCGeoParamsFile2, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->setSecondInput(parIo2);
    rtdb->initContainers(0);

    Double_t fHalfSizeTPC_X = geoPar->GetActiveRegionx() / 2.; // X (row) 		[cm]
    Double_t fHalfSizeTPC_Y = geoPar->GetActiveRegiony() / 2.; // Y (time) 	[cm]
    Double_t fHalfSizeTPC_Z = geoPar->GetActiveRegionz() / 2.; // Z (column) [cm]
    Double_t fSizeOfVirtualPad = geoPar->GetPadSize();         // 1: pads of 1cm^2 , 5: pads of 4mm^2
    Double_t fDriftVelocity = gasPar->GetDriftVelocity();      // [cm/ns]
    Int_t NPads = 128 * 44;

    // Electronics

    Double_t channels = 4096;                                                    // 12 bits electronics
    Double_t dynamic_range = 1.2e-13;                                            // 120fC to detect particle at MIP
    Double_t ADC_Offset = 100;                                                   // To avoid negative ADC values
    Double_t ADC_conv = channels / (dynamic_range / e_equ);                      // converter from n. of e- to ADC value
    Double_t shapingtime = elecPar->GetShapingTime();                            //[ns]
    Double_t TimeBinSize = elecPar->GetTimeBinSize();                            //[ns]
    Double_t NoiseRMS = elecPar->GetNoiseRMS();                                  //[n.e-]
    Double_t thr = (elecPar->GetThreshold()) * NoiseRMS * ADC_conv + ADC_Offset; //[n.e-]
    Double_t Gain = elecPar->GetGain();
    Double_t Theta = elecPar->GetTheta();
    delete electronics;

    cout << "Reading input simulation file " << inputSimFile << endl;
    const int max_time = (int)((2 * fHalfSizeTPC_Y) / fDriftVelocity + 6 * shapingtime);

    cout << " Detector:\033[1;31m " << geoTag << "!!!!!\033[0m" << endl
         << "Values taken for the visualization" << endl
         << "X size (half-length of box): " << fHalfSizeTPC_X << endl
         << "Y size (half-length of box (drift)): " << fHalfSizeTPC_Y << endl
         << "Z size (half-length of box): " << fHalfSizeTPC_Z << endl;
    cout << "\033[1;32m Electronic parameters\033[0m:" << endl
         << "Shaping time: " << shapingtime << " [ns]" << endl
         << "Time bin size: " << TimeBinSize << " [ns]" << endl
         << "Max Drift time: " << max_time << " [ns]" << endl
         << "NoiseRMS: " << NoiseRMS << endl
         << "Threshold: " << thr << " ADC value" << endl
         << "Average Gain for Polya: " << Gain << endl
         << "Theta parameter for Polya: " << Theta << endl;

    // input
    TFile* simFile = TFile::Open(inputSimFile);
    if (simFile->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    TTree* TEvt = (TTree*)simFile->Get("evt");
    Int_t nentries = TEvt->GetEntries(); //
    int eventstart = 0;
    std::cout << "\nTree has " << nentries << " Entries." << std::endl;
    string out = "../AGET/";
    string filename = "output";

    if (argc == 3)
    {
        std::stringstream arg1(argv[1]); // i
        std::stringstream arg2(argv[2]); // total
        int part;
        int partnumber;

        if (arg1 >> part && arg2 >> partnumber)
        {
            if (part > partnumber)
                std::__throw_invalid_argument("#1 > #2");
            std::cout << std::endl << "Engaging Partly analysis ..." << std::endl;
            // Generating output filename
            out += "mt/";
            filename += to_string(part) + "of" + to_string(partnumber);
            // Defining the range of events for the core
            eventstart = (part - 1) * (nentries / partnumber);
            if (partnumber != part)
                nentries = part * (nentries / partnumber);

            std::cout << "Generating File: " << out << filename << ".root"
                      << " as Part " << part << " of " << partnumber << std::endl;
        }
    }
    // output
    TFile MyFileo((out + filename + ".root").c_str(), "RECREATE");
    TTree* tout = new TTree("tout", "Realistic TPC events");
    std::vector<double> x_Pad, z_Pad, t_Pad, q_Pad;
    std::vector<int> pad, eventID, pdgcode, motherid;
    std::vector<int> vertex_PDG, vertex_Mother;
    std::vector<double> x0, y0, z0, px0, py0, pz0;
    std::vector<double> vertex_x0, vertex_y0, vertex_z0, vertex_px0, vertex_py0, vertex_pz0;
    std::vector<double> time;
    TH1D* h_conv;
    TF1 *f_conv, *fit;
    int npads = 0;
    tout->Branch("eventID", &eventID);
    // MCTrack vertex info
    tout->Branch("vertex_PDG", &vertex_PDG);
    tout->Branch("vertex_Mother", &vertex_Mother);
    tout->Branch("vertex_x0", &vertex_x0);
    tout->Branch("vertex_y0", &vertex_y0);
    tout->Branch("vertex_z0", &vertex_z0);
    tout->Branch("vertex_px0", &vertex_px0);
    tout->Branch("vertex_py0", &vertex_py0);
    tout->Branch("vertex_pz0", &vertex_pz0);
    // Pad plane info
    tout->Branch("x_Pad", &x_Pad);
    tout->Branch("z_Pad", &z_Pad);
    tout->Branch("t_Pad", &t_Pad);
    tout->Branch("q_Pad", &q_Pad);

    // GTPCProjPoints
    TClonesArray* gtpcProjPointCA;
    R3BGTPCProjPoint** ppoint;
    gtpcProjPointCA = new TClonesArray("R3BGTPCProjPoint", 5);
    TBranch* branchGTPCProjPoint = TEvt->GetBranch("GTPCProjPoint");
    branchGTPCProjPoint->SetAddress(&gtpcProjPointCA);

    Int_t pointsPerEvent = 0;
    Int_t nb = 0;
    bool PadTouch[NPads]; // Touched pads
    const int samples = (int)(((2 * fHalfSizeTPC_Y) / fDriftVelocity + 6 * shapingtime) / TimeBinSize);
    for (int k = 0; k <= NPads; k++)
    {
        PadTouch[k] = false;
    }
    // Loop over the number of events MT
    std::cout << "\nFrom event ID: " << eventstart << " to " << nentries - 1 << std::endl;

    for (Int_t l = eventstart; l < nentries; l++)
    {
        double percentage = l / (double)(nentries * 1.);
        loadfunction(percentage);
        gtpcProjPointCA->Clear();
        nb += TEvt->GetEvent(l);
        pointsPerEvent = gtpcProjPointCA->GetEntries();

        if (pointsPerEvent > 0)
        {
            ppoint = new R3BGTPCProjPoint*[pointsPerEvent];
            for (Int_t j = 0; j < pointsPerEvent; j++)
            {
                ppoint[j] = new R3BGTPCProjPoint;
                ppoint[j] = (R3BGTPCProjPoint*)gtpcProjPointCA->At(j);
            }

            for (Int_t h = 0; h < pointsPerEvent; h++) // loop over the steps in Active Region
            {
                for (unsigned int i = 0; i < ppoint[h]->GetCharge();
                     i += 1) // loop over the primary e- that reach the pad
                {
                    pdgcode.push_back(ppoint[h]->GetPDGCode());
                    motherid.push_back(ppoint[h]->GetMotherId());
                    x0.push_back(ppoint[h]->GetX0());
                    y0.push_back(ppoint[h]->GetY0());
                    z0.push_back(ppoint[h]->GetZ0());
                    px0.push_back(ppoint[h]->GetPx0());
                    py0.push_back(ppoint[h]->GetPy0());
                    pz0.push_back(ppoint[h]->GetPz0());
                    Int_t padID = ppoint[h]->GetVirtualPadID();
                    PadTouch[padID] = true;
                    pad.push_back(padID);
                    Double_t tPad = ((TH1S*)(ppoint[h]->GetTimeDistribution()))
                                        ->GetMean(); // TODO improve-> one time for each primary e- reaching the pad
                    time.push_back(tPad);
                }
            }
        }
        // NOISE_____________________________________________________________________________________________________________
        // Polya equation MM amplification
        TF1* fonc_gain = new TF1("fonc_gain", polya, 0., 10000., 2);
        fonc_gain->FixParameter(0, Gain);
        fonc_gain->FixParameter(1, Theta);
        double noise[max_time];
        double tab_noise[max_time];
        npads = 0;
        bool tst = false;
        if (pointsPerEvent > 0)
        {
            // Loop over all the pads that are hit during the event ->padID is considered
            for (int i = 0; i <= NPads; i++)
            {

                // Generating the noise
                for (int ii = 0; ii < max_time; ii++)
                {
                    noise[ii] = gRandom->Gaus(0., NoiseRMS);
                }
                // Sampling the noise
                for (int ii = 0; ii < samples; ii++)
                {
                    tab_noise[ii] = noise[(int)(ii * TimeBinSize)];
                }
                double max_noise = tab_noise[TMath::LocMax(samples, tab_noise)];
                double t_max_noise = TMath::LocMax(samples, tab_noise) * TimeBinSize;
                //________________________________________________________________________________________________________________
                char s[10];
                sprintf(s, "%d", i);
                int nb_param = 0;
                // nb_param-> each e- that reachs the pad will increment by 2 the number of parameters:
                // 1 parameter for the charge and 1 for the time
                for (unsigned int j = 0; j < pad.size(); j++)
                {
                    if (pad[j] == i)
                        nb_param += 2;
                }
                // p2:[2*n. of e-, shapingtime, (charge e-)#1, (time e-)#1, (charge e-)#2, ..., (time e-)#n]
                double p2[nb_param + 2];
                p2[0] = nb_param + 2;
                p2[1] = shapingtime;
                // amplification of the signal due to MicroMegas layer
                int ii = 0;
                Int_t xPad, zPad, pdg, mid;
                // vertex info
                Double_t XO, YO, ZO, PXO, PYO, PZO;
                // Looping over the total number of pad hit
                for (unsigned int j = 0; j < pad.size(); j++)
                {
                    if (pad[j] == i)
                    {
                        // Gain Polya distribution
                        double val_gain = fonc_gain->GetRandom();
                        double charge = val_gain;
                        p2[2 * ii + 2 + 1] = time[j] * 1000; // time in nanosec
                        p2[2 * ii + 2] = charge;
                        ii++;
                        // Calculating the padxz
                        xPad = pad[j] % (Int_t)(44);
                        zPad = (pad[j] - xPad) / (44);
                        // vertex info
                        pdg = pdgcode[j];
                        mid = motherid[j];
                        XO = x0[j];
                        YO = y0[j];
                        ZO = z0[j];
                        PXO = px0[j];
                        PYO = py0[j];
                        PZO = pz0[j];
                    }
                }
                // Filling
                // histos_________________________________________________________________________________________________
                h_conv = new TH1D("After shaping", (string(s) + " after shaping").c_str(), samples, 0., max_time);
                f_conv = new TF1("F_conv", conv, 0., max_time, int(p2[0]));

                // N(t)->Signal
                for (int ii = 0; ii < p2[0]; ii++)
                {
                    f_conv->FixParameter(ii, p2[ii]);
                }

                // N(t)+Noise sampled+ADC Offset
                if (PadTouch[i])
                {
                    for (int jj = 0; jj < samples; jj++)
                    {
                        h_conv->Fill(jj * TimeBinSize,
                                     (f_conv->Eval(jj * TimeBinSize)) + noise[(int)(jj * TimeBinSize)] +
                                         (ADC_Offset / ADC_conv));
                    }
                }
                // ADC conversion
                h_conv->Scale(ADC_conv);
                double t_rec = 0.;
                double max, time0;
                Double_t integral;
                if (PadTouch[i])
                {
                    max = h_conv->GetBinContent(h_conv->GetMaximumBin());
                    time0 = h_conv->GetMaximumBin() * TimeBinSize - TimeBinSize / 2.;

                    npads++;
                }
                else
                {
                    max = max_noise;
                    time0 = t_max_noise;
                }
                // Fitting the signal to extract tpad and qpad
                fit = new TF1("FIT", conv_fit, 0., max_time, 4);
                fit->SetParLimits(0, 0., 100000000);
                fit->SetParameter(0, max);
                fit->SetParLimits(1, time0 - shapingtime * 2., time0);
                fit->SetParameter(1, time0 - shapingtime);
                fit->FixParameter(2, p2[1]); // shaping time
                fit->SetParameter(3, 0.);    // offset
                fit->SetLineColor(2);

                if (PadTouch[i])
                {
                    h_conv->Fit(fit, "NOQ"); // Q=quiet
                    t_rec = fit->GetParameter(1);
                    integral = fit->GetMaximum(0., max_time);
                }

                // electronics readout
                if (max > (thr) && PadTouch[i])
                {
                    x_Pad.push_back(xPad);  // x coordinate in number of pads
                    z_Pad.push_back(zPad);  // z coordinate in number of pads
                    t_Pad.push_back(t_rec); // time coordinate of the pads (us)
                    if (integral > 4096)
                        q_Pad.push_back(4096); // saturation of the electronics
                    else
                        q_Pad.push_back(integral); // charge deposited on the pads
                    eventID.push_back(l);          // ID of the event
                    vertex_PDG.push_back(pdg);
                    vertex_Mother.push_back(mid);
                    vertex_x0.push_back(XO);
                    vertex_y0.push_back(YO);
                    vertex_z0.push_back(ZO);
                    vertex_px0.push_back(PXO);
                    vertex_py0.push_back(PYO);
                    vertex_pz0.push_back(PZO);
                }
                delete h_conv;
                delete f_conv;
                delete fit;
            } // end of (l) loop
        }
        tout->Fill();
        for (int i = 0; i <= NPads; i++)
        {
            PadTouch[i] = false;
        }
        // Clear the vectors before going to the next event
        time.clear();
        pdgcode.clear();
        motherid.clear();
        x0.clear();
        y0.clear();
        z0.clear();
        px0.clear();
        py0.clear();
        pz0.clear();
        vertex_PDG.clear();
        vertex_Mother.clear();
        vertex_x0.clear();
        vertex_y0.clear();
        vertex_z0.clear();
        vertex_px0.clear();
        vertex_py0.clear();
        vertex_pz0.clear();
        pad.clear();
        eventID.clear();
        x_Pad.clear();
        z_Pad.clear();
        t_Pad.clear();
        q_Pad.clear();
    }

    cout << "\n" << endl;
    MyFileo.Write();
    MyFileo.Close();
    timer.Stop();
    cout << "\nReal time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}

void loadfunction(double& percentage)
{
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
// functions for the electronics response
double polya(double x[], double p[]) // Gain
{
    double val =
        (pow(p[1] + 1, p[1] + 1) / TMath::Gamma(p[1] + 1)) * pow(x[0] / p[0], p[1]) * exp(-(p[1] + 1) * x[0] / p[0]);
    return val;
}
double conv(double x[], double p[]) // pad response
{
    double val = 0.;
    for (int i = 0; i < (p[0] - 2) / 2; i++)
    {
        if (!(x[0] < p[2 * i + 2 + 1] || x[0] > 1000000.))
            val += p[2 * i + 2] * 22.68113723 * exp(-3. * (x[0] - p[2 * i + 2 + 1]) / p[1]) *
                   sin((x[0] - p[2 * i + 2 + 1]) / p[1]) * pow((x[0] - p[2 * i + 2 + 1]) / p[1], 3);
    }
    return (val);
}
double conv_fit(double x[], double p[]) // signal fit
{
    double val = 0.;
    if (!(x[0] < p[1] || x[0] > 1000000.))
        val += p[0] * 22.68113723 * exp(-3. * (x[0] - p[1]) / p[2]) * sin((x[0] - p[1]) / p[2]) *
                   pow((x[0] - p[1]) / p[2], 3) +
               p[3];
    else
        val += p[3];
    return (val);
}
