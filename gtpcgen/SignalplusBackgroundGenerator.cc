/*
AUTHOR: SIMONE VELARDITA
 Creation date:06-July-2021
 Backup:none
HOW TO USE
 root [0] .L SignalplusBackgroundGenerator.cc+
 root [1] SignalplusBackgroundGenerator()*/

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGenPhaseSpace.h"
#include "TH1.h"
#include "TH2.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TTree.h"
#include "TVector3.h"

#include <Riostream.h>
#include <TString.h>

// C++ Lybrary
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdlib.h> 
#include <string>
#include <vector>
#include <sys/stat.h>
using namespace std;
// For flight decay vector

double tauHyper = 0.216;  // 3LH decay time in ns-> world average C. Rappold et al. Physics Letters B 728 (2014) 543–548
double tauLambda = 0.263; // Lambda decay time in ns-> 0.263
double c = 29.9792; 			// speed of light cm/ns
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

void SignalplusBackgroundGenerator()
{
		string GEOTAG = "Prototype";
    // For the particle position
    TRandom3 r3(0);
		cout<<"TO BE VALIDATE!!!!!!!!!!!"
    // Open the NHL file
    TFile* fopen = new TFile("./root/input_beam_dcm_C12C12_1.9.root");
    if (fopen->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);

    TTree* t = (TTree*)fopen->Get("tree_input");
    // Declar and set the branch address
    Int_t nmax = 40;                   // max dimension of the array, value took from TTree::MakeClass
    Short_t nParticles;                // number of particles after the collision
    Short_t A[nmax], Z[nmax], S[nmax]; // characteristic of the produced particles after the collision
    Float_t px[nmax], py[nmax], pz[nmax], EKin[nmax];
    Int_t PDGCode[nmax];
    t->SetBranchAddress("nParticles", &nParticles);
    t->SetBranchAddress("A", A);
    t->SetBranchAddress("Z", Z);
    t->SetBranchAddress("S", S);				//Strangness
    t->SetBranchAddress("PDGCode", PDGCode);
    t->SetBranchAddress("px", px);
    t->SetBranchAddress("py", py);
    t->SetBranchAddress("pz", pz);
    t->SetBranchAddress("EKin", EKin);
    //Check if the ASCII folder is present
    string folder="ASCII";
		if (mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
		{
    	if( errno == EEXIST ) 
    	{
				cout<<"The folder exists!!"<<endl;
    	} 
    	else 
    	{
        std::cout << "cannot create session name folder error:" << strerror(errno) << std::endl;
        throw std::runtime_error( strerror(errno) );
    	}
		}
		// Create the ASCIIfile that will be used by the simulation
    ofstream asciifile;
    //Target position according to the different detectors
    TVector3 TargetPosition;
    if (GEOTAG == "Prototype")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputPrototype_SignalBackground.dat", ios::out);
        TargetPosition = {-2.46, 0., 222.7};//-6.12, 0., 260.0
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
    if (GEOTAG == "FullBeamOut")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputFullBeamOut_SignalBackground.dat", ios::out);
        TargetPosition = { 0, 0., 147.5 }; 
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
    if (GEOTAG == "FullBeamIn")
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        asciifile.open("./ASCII/inputFullBeamIn_SignalBackground.dat", ios::out);
        TargetPosition = { 0, 0., 170};//161, 167.5, 172.5 155 145
        cout << "The target Position is (" << TargetPosition.X() << ", " << TargetPosition.Y() << ", "
             << TargetPosition.Z() << ") [cm]" << endl;
    }
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
    // We have to add the fly time for the Lamba and H3; The masses has to be in GeV
    int Ndecay = 2;
    const Double_t MassHyper[] = { 2.80839, 0.13957 };  // 3He,pi-
    const Double_t MassLambda[] = { 0.938272, 0.13957 }; // proton pi-
    int hyper_flag;
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
    // Now we have to loop inside the NHL file
    Int_t Nentr = (Int_t)t->GetEntries();
    cout << "Total number of entries:	" << Nentr << endl;
    double chyper=0,clambda=0;
    
    for (int i = 0; i < Nentr; ++i)
    {
    		double percentage=i/(double)(Nentr*1.0);
				loadfunction(percentage);

				float TargetRadius = 0.25, TargetLength = 5.0;//before radius was 25mm
   			float FWHM=0.4;								//2 mm
   			float sig_x = FWHM/2.355, sig_y = FWHM/2.355;

			  float tgtx = -999, tgty = -999;
			  while(sqrt(tgtx*tgtx+tgty*tgty)>TargetRadius)
			  {
				   tgtx = r3.Gaus(0, sig_x);
				   tgty = r3.Gaus(0, sig_y);
			  }
			  float tgtz = r3.Uniform(-0.5*TargetLength, 0.5*TargetLength);
			  TVector3 pos_vec, pos_decay;
			  pos_vec.SetXYZ(tgtx, tgty, tgtz);
			  pos_vec+=TargetPosition;
			  pos_decay=pos_vec;
			  hyper_flag=0;
        
        // Now for each event we have to loop on the number of particles produced after the collision
        t->GetEntry(i);
        int Nsize = nParticles;
        float Mass[Nsize];
        TLorentzVector P[Nsize];
				int multiplicity=nParticles;
        int eventNumber = i;
        for (int l = 0; l < Nsize; ++l)
        {
        	if (PDGCode[l] == 1010010030) multiplicity++;
        	if (PDGCode[l] == 3122) multiplicity++;
        }
        // Event header composite by: #eventNumber #multiplicity 0. 0.        
       	if (asciifile.is_open())
        {
            asciifile << eventNumber << "  " << multiplicity << "  0.  0."<< "\n";
        }
        for (int j = 0; j < Nsize; ++j)
        {
         if (PDGCode[j] != 3122 && PDGCode[j] != 1010010030)
         {
		      Mass[j] = (0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j])*0.001;//[GeV]
      		P[j].SetPxPyPzE(px[j]*0.001, py[j]*0.001, pz[j]*0.001, (EKin[j] + Mass[j])*0.001);
				  if (asciifile.is_open())
				  {
				  	if (PDGCode[j]<10e8)//Elementary particles
				    {
				    	asciifile << "1  0		"<<PDGCode[j]<<"  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
				                << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
				                <<Mass[j]
				                << "\n";
				    }
				    if (PDGCode[j] >= 10e8)//IONS
				    {
				     	asciifile << "-1  "<<Z[j]<<"		"<<A[j]<<"  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
				                << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
				                << Mass[j]
				                << "\n";
				    }
				   }
    			}
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
          if (PDGCode[j] == 1010010030)
            {
            		chyper++;
                TGenPhaseSpace Decay;
                Int_t Zdecay[] = { 2, -1 };
                Int_t Adecay[] = { 3, 0 };
                //cout<<endl<<"You have found an Hypertriton. Event N. "<<i<<endl;
                Double_t Tbeam = EKin[j]*0.001; // Ekin
                Mass[j] = (0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j])*0.001;//[GeV]
                Double_t Ebeam = EKin[j] + Mass[j]; // Ekin+Mass
                Double_t Pbeam_abs = sqrt(EKin[j] * (EKin[j] + 2.0 * Mass[j]));
                //cout<<"Theoretical mass Hypertriton=2992.14 MeV . With the Formula M= "<<Mass[j]<<" MeV"<<endl;
                double beta = Pbeam_abs / Ebeam;
                double Gamma = 1 / sqrt(1 - (beta * beta));
                //cout<<"Pbeam= "<<Pbeam_abs<<"	Ebeam= "<<Ebeam<<"	beta=	"<<beta<<"	Gamma=	"<<Gamma<<endl;
                TLorentzVector PHyper(px[j]*0.001, py[j]*0.001, pz[j]*0.001, Ebeam);
                Decay.SetDecay(PHyper, Ndecay, MassHyper);

                // Generate decayed events
                double weight = Decay.Generate();
                while (gRandom->Uniform() > weight)
                    weight = Decay.Generate();

                // Flight decay vector generator
                TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tauHyper);
                f1->SetParameter(0, Gamma * tauHyper);
                Double_t t = f1->GetRandom();
                Double_t l = (Gamma * beta * c * t);
                TVector3 d_length(0, 0, l);
                pos_decay += d_length;
                // The Hypertriton decay
                TLorentzVector Pdecay[Ndecay];
                for (int idecay = 0; idecay < Ndecay; ++idecay)
                {
                  Pdecay[idecay] = *(Decay.GetDecay(idecay));
                	if (asciifile.is_open())
									{
										if (Zdecay[idecay] == -1)//pi-
										{
											asciifile << "1  0	-211	"<< Pdecay[idecay].Px() << "		" << Pdecay[idecay].Py() << "		" << Pdecay[idecay].Pz() << "		"
												        << pos_decay.x() << "		" << pos_decay.y() << "		" << pos_decay.z() << "		"
												        <<MassHyper[idecay]
												        << "\n";
										}
										if (Zdecay[idecay] == 2 && Adecay[idecay] == 3)//He3
										{
										 	asciifile << "-1		"<<Zdecay[idecay]<<"		"<<Adecay[idecay]<<"		" 
										 						<< Pdecay[idecay].Px() << "		" << Pdecay[idecay].Py() << "		" << Pdecay[idecay].Pz() << "		"
												        << pos_decay.x() << "		" << pos_decay.y() << "		" << pos_decay.z() << "		"
												        << MassHyper[idecay]
												        << "\n";
										}
									 }
                }
            }
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
            if (PDGCode[j] == 3122)
            {
            		clambda++;
                //cout<<endl<<"You have found a free Lambda. Event N. "<<i<<endl;
                hyper_flag = 1;
                TGenPhaseSpace Decay;
                Int_t Zdecay[] = { 1, -1 };
                Int_t Adecay[] = { 1, 0 };
                Double_t Tbeam = EKin[j]; // Ekin
                Mass[j] = 0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j];
                // cout<<"Theoretical mass Lamda=1115.683 MeV. With the Formula M= "<<Mass[j]<<" MeV"<<endl;
                Double_t Ebeam = EKin[j] + Mass[j]; // Ekin+Mass
                Double_t Pbeam_abs = sqrt(EKin[j] * (EKin[j] + 2.0 * Mass[j]));

                double beta = Pbeam_abs / Ebeam;
                double Gamma = 1 / sqrt(1 - (beta * beta));
                // cout<<"Pbeam= "<<Pbeam_abs<<"	Ebeam= "<<Ebeam<<"	beta=	"<<beta<<"	Gamma=	"<<Gamma<<endl;
                TLorentzVector PLambda(px[j], py[j], pz[j], Ebeam);
                Decay.SetDecay(PLambda, Ndecay, MassLambda);

                // Generate decayed events
                double weight = Decay.Generate();
                while (gRandom->Uniform() > weight)
                    weight = Decay.Generate();

                // Flight decay vector generator
                TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tauLambda);
                f1->SetParameter(0, Gamma * tauLambda);
                Double_t t = f1->GetRandom();
                Double_t l = (Gamma * beta * c * t);
                TVector3 d_length(0, 0, l);
                pos_decay += d_length;
                // The Lambda decay
                TLorentzVector Pdecay[Ndecay];
                for (int idecay = 0; idecay < Ndecay; ++idecay)
                {
                  Pdecay[idecay] = *(Decay.GetDecay(idecay));
                	if (asciifile.is_open())
									{
										if (Zdecay[idecay] == -1)//pi-
										{
											asciifile << "1		0		-211		"<< Pdecay[idecay].Px() << "		" << Pdecay[idecay].Py() << "		" << Pdecay[idecay].Pz() << "		"
												        << pos_decay.x() << "		" << pos_decay.y() << "		" << pos_decay.z() << "		"
												        <<MassLambda[idecay]
												        << "\n";
										}
										if (Zdecay[idecay] == 1 && Adecay[idecay] == 1)//proton
										{
										 	asciifile << "1		0		2212		"<< Pdecay[idecay].Px() << "		" << Pdecay[idecay].Py() << "		" << Pdecay[idecay].Pz() << "		"
												        << pos_decay.x() << "		" << pos_decay.y() << "		" << pos_decay.z() << "		"
												        << MassLambda[idecay]
												        << "\n";
										}
									 } 
                }
            }
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
        }//size
    }//entries
    double inelastic_cross_section=909.148; //[mb]
    double N_collision=1E8;
    double cross_section_hyper=chyper*inelastic_cross_section/N_collision;
    double cross_section_lambda=clambda*inelastic_cross_section/N_collision;
    cout<<endl<<"The hypertriton cross section production is: "<<cross_section_hyper<<"	[mb]	for a total of hyper="<<chyper<<endl;
    cout<<endl<<"The Free Lambda cross section production is: "<<cross_section_lambda<<"	[mb]	for a total of lambda="<<clambda<<endl;
    // write and close everything
    fopen->Close();
    asciifile.close();
    cout<<"\n"<<endl;
}
/*
FROM YELEI's EMAIL
The scale for the calculation of the cross section is:
N*inelastic_cross_section/N_collision
N is the particle produced
inelastic_cross_section is inelastic cross section, 909.148mb fro 12C+12C at 1.9GeV/nculeon
N_collision is number of inelastic collisions,  it is 1E8 for the current case.
*/
