/*
AUTHOR: SIMONE VELARDITA
 Creation date:27-Jan-2021
HOW TO USE
 root [0] .L INCL_Background_ASCIIGenerator.cc+
 root [1] Generate_background()*/

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
#include "TString.h"
#include "TSystem.h"
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
using namespace std;
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

void Generate_background() 
{

  TRandom3 r3(0);
  ofstream asciifile;
  // choose the detector
  const char* geoTag = "Prototype";
	const char* geoTag1 = "FullBeamOut";
	const char* geoTag2 = "FullBeamIn";
	TString GEOTAG = string(geoTag);

  // Open the NHL file
  TFile *fopen = new TFile("./root/C12_C12_22800_de-excitation=abla07.root");
  if (fopen->IsOpen())
    cout << "File correctly opened!" << endl;
  else
    exit(1);
  // Take the tree
  TTree *t = (TTree *)fopen->Get("et");
  // Mx dimension of the array, value took from TTree::MakeClass
  Int_t nmax = 40;
  // number of particles after the collision
  Short_t nParticles;
  // characteristics of the produced particles after the collision
  Short_t A[nmax], Z[nmax];
  Float_t px[nmax], py[nmax], pz[nmax], EKin[nmax];
  Int_t PDGCode[nmax];
  t->SetBranchAddress("nParticles", &nParticles);
  t->SetBranchAddress("A", A);
  t->SetBranchAddress("Z", Z);
  t->SetBranchAddress("PDGCode", PDGCode);
  t->SetBranchAddress("px", px);
  t->SetBranchAddress("py", py);
  t->SetBranchAddress("pz", pz);
  t->SetBranchAddress("EKin", EKin);

  //Target position according to the different detector
  TVector3 TargetPosition;
  if (GEOTAG.CompareTo("Prototype") == 0) {
    cout << "\033[1;31m Warning\033[0m: The detector is-> " << GEOTAG << endl;
    asciifile.open("./ASCII/inputPrototype_bkg.dat", ios::out | ios::app);
    TargetPosition = {-0.7, 0., 192.0};
    cout << "The target Position is (" << TargetPosition.X() << ", "
         << TargetPosition.Y() << ", " << TargetPosition.Z() << ") [cm]"
         << endl;
  }
  if (GEOTAG.CompareTo("FullBeamOut") == 0) {
    cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
    asciifile.open("./ASCII/inputFullBeamOut_bkg.dat", ios::out | ios::app);
    TargetPosition = {0, 0., 0}; // TODO add the coordinates
    cout << "The target Position is (" << TargetPosition.X() << ", "
         << TargetPosition.Y() << ", " << TargetPosition.Z() << ") [cm]"
         << endl;
  }
  if (GEOTAG.CompareTo("FullBeamIn") == 0) {
    cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
    asciifile.open("./ASCII/inputFullBeamIn_bkg.dat", ios::out | ios::app);
    TargetPosition = { 0, 0., 161. };
    cout << "The target Position is (" << TargetPosition.X() << ", "
         << TargetPosition.Y() << ", " << TargetPosition.Z() << ") [cm]"
         << endl;
  }

  // Now we have to loop inside the NHL file
  Int_t Nentr = (Int_t)t->GetEntries();
  cout << "Total number of entries inside " << Nentr << endl;
  
  //____________________________________________________
  for (int i = 0; i < Nentr; ++i) {
  	
  	double percentage=i/(double)Nentr*1.;
  	loadfunction(percentage);

		//Vertex position
    float TargetRadius = 0.25, TargetLength = 5.0;
    float FWHM = 0.4;
    float sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
    float tgtx = -999, tgty = -999;
    while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius) {
      tgtx = r3.Gaus(0, sig_x);
      tgty = r3.Gaus(0, sig_y);
    }
    float tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
    TVector3 pos_vec;
    pos_vec.SetXYZ(tgtx, tgty, tgtz);
    pos_vec += TargetPosition;

    t->GetEntry(i);
    int Nsize = nParticles; // multiplicity
    float Mass[Nsize];
    TLorentzVector P[Nsize];
    // asciifile header-> #event	multiplicity	0.	0.
    int eventNumber = i;
    if (asciifile.is_open()) {
      asciifile << eventNumber << "  " << Nsize << "  0.  0."
                << "\n";
    }

    for (int j = 0; j < Nsize; ++j) {
      Mass[j] =
          (0.5 *
          (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) /
          EKin[j])*0.001;//[GeV]
      P[j].SetPxPyPzE(px[j]*0.001, py[j]*0.001, pz[j]*0.001, (EKin[j] + Mass[j])*0.001);
      if (asciifile.is_open()) {
        if (Z[j] == -1) 
        {
          asciifile << "1  0  " << PDGCode[j] << "  " << P[j].Px() << "  "
                    << P[j].Py() << "  " << P[j].Pz() << "  " << pos_vec.x()
                    << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                    << Mass[j] << "\n";
        }
        if (Z[j] == 0) {
          asciifile << "1  0  " << PDGCode[j] << "  " << P[j].Px() << "  "
                    << P[j].Py() << "  " << P[j].Pz() << "  " << pos_vec.x()
                    << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                    << Mass[j] << "\n";
        }
        if (Z[j] == 1) {
          if (PDGCode[j] == 2212 || PDGCode[j] == 211 || PDGCode[j] == 321 ||
              PDGCode[j] == 3222) {
            asciifile << "1  0  " << PDGCode[j] << "  " << P[j].Px() << "  "
                      << P[j].Py() << "  " << P[j].Pz() << "  " << pos_vec.x()
                      << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                      << Mass[j] << "\n";
          } else
            asciifile << "-1  " << Z[j] << "	" << A[j] <<"  " 
            					<< P[j].Px() << "  " << P[j].Py()
                      << "  " << P[j].Pz() << "  " << pos_vec.x() << "  "
                      << pos_vec.y() << "  " << pos_vec.z() << "  " << Mass[j]
                      << "\n";
        }
        if (Z[j] > 1)
          asciifile << "-1  " << Z[j] << "	" << A[j] << "  "
          					<< P[j].Px() << "  " << P[j].Py()
                    << "  " << P[j].Pz() << "  " << pos_vec.x() << "  "
                    << pos_vec.y() << "  " << pos_vec.z() << "  " << Mass[j]
                    << "\n";
      }
    }
  }
  // Closing the files
  asciifile.close();
  fopen->Close();
  cout<<"\n"<<endl;
}
