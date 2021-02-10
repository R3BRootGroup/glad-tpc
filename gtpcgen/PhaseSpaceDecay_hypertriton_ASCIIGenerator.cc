/*
 AUTHOR: SIMONE VELARDITA
 Creation date:27-Jan-2021
 HOW TO USE
 Choose the right detector for GEOTAG
 root [0] .L PhaseSpaceDecay_hypertriton_ASCIIGenerator.cc+
 root [1] GenerateInputTree_PhaseSpaceDecay()*/

/*
To create the ASCII file for R3BROOT everything must be in [ns], [cm] and [GeV]
*/
#include "TCanvas.h"
#include "TF1.h"
#include "TF2.h"
#include "TFile.h"
#include "TGenPhaseSpace.h"
#include "TH1.h"
#include "TH2.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TVector3.h"

// C++ Lybrary
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdlib.h> 
#include <string>
#include <vector>
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

using namespace std;

// For flight decay vector
double tau = 0.216; // 3LH decay time in ns-> world average C. Rappold et al. Physics Letters B 728 (2014) 543–548
double c = 29.9792; // speed of light cm/ns
string GEOTAG;

void GenerateInputTree_PhaseSpaceDecay()
{
    const char* geoTag = "Prototype";
    const char* geoTag1 = "FullBeamOut";
    const char* geoTag2 = "FullBeamIn";
    GEOTAG = std::string(geoTag1);
    // Number of total decays 
    const int neve = 10000; //select the total number of events that you need
		
		//Decayed particles characteristics
    TRandom3 r3(0), E(0);
    const Int_t Abeam = 3;
    const Int_t Afrag = 3;
    const Int_t Zfrag = 2;
    const Int_t Ndecay = 2;
    const Int_t Z[Ndecay] = { Zfrag, -1 };
    const Int_t A[Ndecay] = { Afrag, 0 };
    const Double_t Masses[Ndecay] = { 2.80839, 0.13957 }; // 3He,pi
    Double_t E_in = 2.; // Beam incident energy GeV/nucleon for Hypertriton decay
    
    // Init Phase Space Decay
    double Mbeam = 2.99214; // 3LH -> D.H.Davis Nucl. Phys. A 754 (2006) 3
    TGenPhaseSpace Decay;
		//output file
    ofstream asciifile;
    //Target position according to the different detectors
    TVector3 TargetPosition;
    if (GEOTAG == "Prototype")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputPrototype_3LH.dat", ios::out | ios::app);
        TargetPosition = { -0.7, 0., 192.0 };
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
    if (GEOTAG == "FullBeamOut")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputFullBeamOut_3LH.dat", ios::out | ios::app);
        TargetPosition = { 0, 0., 147.5 }; 
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
    if (GEOTAG == "FullBeamIn")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputFullBeamIn_3LH.dat", ios::out | ios::app);
        TargetPosition = { 0, 0., 161. };
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
		cout<<"Number of hypertriton decay events: "<<neve<<endl;
    //____________________________________________________
    for (int i = 0; i < neve; ++i)
    {
    		double percentage=i/10000.;
				loadfunction(percentage);

        double sig_E = 0.1344, mean_E = 1.555; //[GeV]
        double E_in = E.Gaus(mean_E, sig_E);

        Double_t Tbeam = Abeam * E_in;
        Double_t Ebeam = Mbeam + Tbeam;
        Double_t Pbeam_abs = sqrt(Tbeam * (Tbeam + 2.0 * Mbeam));

        TLorentzVector Pbeam(0, 0, Pbeam_abs, Ebeam);

        double TargetRadius = 0.25, TargetLength = 5.0;
        double FWHM = 0.4;
        double sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
        double tgtx = -999, tgty = -999;
        while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius)
        {
            tgtx = r3.Gaus(0, sig_x);
            tgty = r3.Gaus(0, sig_y);
        }
        double tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
        TVector3 pos_vec;
        pos_vec.SetXYZ(tgtx, tgty, tgtz);
        pos_vec += TargetPosition;

        // angular spread[rad], deduced from Dubna cascade model+fermi break-up
        Double_t xang = gRandom->Landau(0.016, 0.006); // theta
        Double_t yang = gRandom->Landau(-0.29, 0.8);   // phi

        Pbeam.SetTheta(xang);
        Pbeam.SetPhi(yang);

        Decay.SetDecay(Pbeam, Ndecay, Masses);

        // Generate decayed events
        double weight = Decay.Generate();
        while (gRandom->Uniform() > weight)
            weight = Decay.Generate();

        // Flight decay vector generator
        double beta = Pbeam_abs / Ebeam;
        double Gamma = 1 / sqrt(1 - (beta * beta));
        
        TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 12 * tau);
        f1->SetParameter(0, Gamma * tau);
        Double_t t = f1->GetRandom();

        Double_t l = (Gamma * beta * c * t);

        TVector3 d_length(0, 0, l);
        pos_vec += d_length;

        // Carbon12 beam
        double MC = 11.200;     // 12u->1u=931.50*MeV/c^2
        Double_t TC = 12 * 1.9; // 1.9*GeV/A
        Double_t EC = MC + TC;
        Double_t PC_abs = sqrt(TC * (TC + 2.0 * MC));
        TLorentzVector PC(0, 0, PC_abs, EC);
        
        // Event header composite by: #eventNumber #multiplicity 0. 0.
        int multiplicity = 3;// fixed:2 from the decay and 1 is the beam
        int eventNumber = i;
        
        if (asciifile.is_open())
        {
            asciifile << eventNumber << "  " << multiplicity << "  0.  0."
                      << "\n";
            // the position for the 12C beam is fixed: -2m
            asciifile << "-1  6  12  " << PC.Px() << "  " << PC.Py() << "  " << PC.Pz() << "  0.  0.  -200.  11.178"
                      << "\n";
        }
        TLorentzVector P[Ndecay];
        for (int j = 0; j < Ndecay; ++j)
        {
            P[j] = *(Decay.GetDecay(j));
        }
        for (int j = 0; j < Ndecay; ++j)
        {
            if (asciifile.is_open())
            {
                if (Z[j] == -1)//pi-
                {
                    asciifile << "1  0  -211  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
                              << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                              << "  0.140"
                              << "\n";
                }
                if (Z[j] == 2)//He3
                {
                    asciifile << "-1  2  3  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
                              << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                              << "  2.808"
                              << "\n";
                }
            }
        }
    }

    asciifile.close();
    cout<<"\n"<<endl;
}

/*
    //Benchmark beam-> 1GeV proton beam it's expected to be bent 51 degure
    double Mp = 938.272;//MeV/c^2
    Double_t Tp = 1000;//1.*GeV/A
    Double_t Ep = Mp + Tp;
    Double_t Pp_abs = sqrt(Tp*(Tp+2.0*Mp));
    TLorentzVector Pp(0, 0, Pp_abs, Ep);
    TBeamSimData particle(1,0,Pp,TVector3(0,0,-2000));//-2*m
    particle.fParticleName="proton";
    gBeamSimDataArray->push_back(particle);*/
