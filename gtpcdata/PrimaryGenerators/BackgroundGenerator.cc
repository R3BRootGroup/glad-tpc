/*
AUTHOR: SIMONE VELARDITA
 Creation date:22-May-2020
 Backup:none
HOW TO USE
 root [0] .L BackgroundGenerator.cc+
 root [1] BackgroundGenerator()*/

#include "TBeamSimData.hh"
#include "TBenchmark.h"
#include "TRunSimParameter.hh"
#include "TSimData.hh"
#include "TSystem.h"

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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <string>
#include <vector>
using namespace std;

void BackgroundGenerator()
{

    // For the particle position
    TRandom3 r3(0);

    // Open the NHL file
    TFile* fopen = new TFile("C12_C12_22800_de-excitation=abla07.root");
    if (fopen->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    // Take the tree
    TTree* t = (TTree*)fopen->Get("et");
    // Declar and set the branch address
    Int_t nmax = 40;          // max dimension of the array, value took from TTree::MakeClass
    Short_t nParticles;       // number of particles after the collision
    Short_t A[nmax], Z[nmax]; // characteristic of the produced particles after the collision
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
    // Create the Root file and tree that will be used by the simulation
    TFile* file = new TFile("../root/inputtree_BKG.root", "recreate");
    TTree* tree = new TTree("tree_input", "Input tree for Background");
    gBeamSimDataArray = new TBeamSimDataArray();
    tree->Branch("TBeamSimData", gBeamSimDataArray);

    // Now we have to loop inside the NHL file
    Int_t Nentr = (Int_t)t->GetEntries();
    cout << "Total number of entries inside " << Nentr << endl;

    for (int i = 0; i < Nentr; ++i)
    {
        gBeamSimDataArray->clear();

        //______TARGET_____________________________________________________everything is in mm (don't modify the
        //units!!!)

        float TargetRadius = 5., TargetLength = 40.; // before radius was 25mm
        // TVector3 TargetPosition(-6.75,0.,1850.);//prototype-old
        // TVector3 TargetPosition(-7,0.,1920.);//prototype
        TVector3 TargetPosition(0, 0., 1450.); // Full HYDRA
        float FWHM = 2;                        // 2 mm
        float sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
        // cout<<"sigma x= "<<sig_x<<"	sigma y= "<<sig_y<<endl;
        float tgtx = -999, tgty = -999;
        while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius)
        {
            tgtx = r3.Gaus(0, sig_x);
            tgty = r3.Gaus(0, sig_y);
        }
        float tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
        TVector3 pos_vec;
        pos_vec.SetXYZ(tgtx, tgty, tgtz);
        pos_vec += TargetPosition;

        // Now for each event we have to loop on the number of particles produced after the collision
        t->GetEntry(i);
        int Nsize = nParticles;
        float Mass[Nsize];
        TLorentzVector P[Nsize];
        for (int j = 0; j < Nsize; ++j)
        {
            Mass[j] = 0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j];
            // if(j<10) cout<<"Massa= "<<Mass[j]<<endl;
            P[j].SetPxPyPzE(px[j], py[j], pz[j], EKin[j] + Mass[j]);
            TBeamSimData particle(Z[j], A[j], P[j], pos_vec);
            particle.fPDGCode = PDGCode[j];
            if (Z[j] == 0)
                particle.fParticleName = "neutron";
            if (Z[j] == 1 && A[j] == 0)
                particle.fParticleName = "proton";
            if (Z[j] > 1)
                particle.fParticleName = "";
            if (Z[j] == -1)
                particle.fParticleName = "pi-";
            if (PDGCode[j] == 211)
                particle.fParticleName = "pi+";
            if (PDGCode[j] == -321)
                particle.fParticleName = "kaon-";
            if (PDGCode[j] == 321)
                particle.fParticleName = "kaon+";

            particle.fPrimaryParticleID = i;
            gBeamSimDataArray->push_back(particle);
        }

        tree->Fill();
    }
    // write and close everything
    tree->Write();
    file->Close();

    fopen->Close();
}
