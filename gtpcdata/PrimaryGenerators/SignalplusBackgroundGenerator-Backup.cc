/*
AUTHOR: SIMONE VELARDITA
 Creation date:29-May-2020
 Backup:none
HOW TO USE
 root [0] .L SignalplusBackgroundGenerator.cc+
 root [1] SignalplusBackgroundGenerator()*/

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
// For flight decay vector

double tauHyper = 0.216; // 3LH decay time in ns-> world average C. Rappold et al. Physics Letters B 728 (2014) 543–548
double tauLambda = 0.263; // Lambda decay time in ns-> 0.263
double c = 299.792;       // mm/ns

// Here, we will declare a function that allow us to get the daughters, passing just few preliminar value
// just for now it will be
void GetDaughter();

void SignalplusBackgroundGenerator()
{

    // For the particle position
    TRandom3 r3(0);

    // Open the NHL file
    TFile* fopen = new TFile("input_beam_dcm_C12C12_1.9.root");
    if (fopen->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    // Take the tree
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
    t->SetBranchAddress("S", S);
    t->SetBranchAddress("PDGCode", PDGCode);
    t->SetBranchAddress("px", px);
    t->SetBranchAddress("py", py);
    t->SetBranchAddress("pz", pz);
    t->SetBranchAddress("EKin", EKin);
    // Create the Root file and tree that will be used by the simulation
    TFile* file = new TFile("../root/inputtree_ALL.root", "recreate");
    TTree* tree = new TTree("tree_input", "Input tree for Signal plus Background");
    gBeamSimDataArray = new TBeamSimDataArray();
    tree->Branch("TBeamSimData", gBeamSimDataArray);
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
    // We have to add the fly time for the Lamba and H3
    int Ndecay = 2;
    const Double_t MassHyper[Ndecay] = { 2808.39, 139.57 };  // 3He,pi-
    const Double_t MassLambda[Ndecay] = { 938.272, 139.57 }; // proton pi-
    int hyper_flag;
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
    // Now we have to loop inside the NHL file
    Int_t Nentr = (Int_t)t->GetEntries();
    cout << "Total number of entries inside " << Nentr << endl;
    for (int i = 0; i < Nentr; ++i)
    {
        gBeamSimDataArray->clear();

        //______TARGET_____________________________________________________everything is in mm (don't modify the
        //units!!!)

        float TargetRadius = 5., TargetLength = 40.; // before radius was 25mm
        TVector3 TargetPosition(-6.75, 0., 1850.);   // Prototype
        // TVector3 TargetPosition(0,0.,1425.);//HYDRA
        float FWHM = 2; // 2 mm
        float sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
        // cout<<"sigma x= "<<sig_x<<"	sigma y= "<<sig_y<<endl;
        float tgtx = -999, tgty = -999;
        while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius)
        {
            tgtx = r3.Gaus(0, sig_x);
            tgty = r3.Gaus(0, sig_y);
        }
        float tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
        TVector3 pos_vec, pos_decay;
        pos_vec.SetXYZ(tgtx, tgty, tgtz);
        pos_vec += TargetPosition;
        pos_decay = pos_vec;
        hyper_flag = 0;
        // Now for each event we have to loop on the number of particles produced after the collision
        t->GetEntry(i);
        int Nsize = nParticles;
        float Mass[Nsize];
        TLorentzVector P[Nsize];
        for (int j = 0; j < Nsize; ++j)
        {

            if (PDGCode[j] != 3122 && PDGCode[j] != 1010010030)
            {
                Mass[j] = 0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j];
                P[j].SetPxPyPzE(px[j], py[j], pz[j], EKin[j] + Mass[j]);
                TBeamSimData particle(Z[j], A[j], P[j], pos_vec);
                particle.fPDGCode = PDGCode[j]; // 1010010030->Hypertriton && 3122->Lambda
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
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
            if (PDGCode[j] == 1010010030)
            {

                TGenPhaseSpace Decay;
                Int_t Zdecay[Ndecay] = { 2, -1 };
                Int_t Adecay[Ndecay] = { 3, 0 };
                // cout<<"You found an Hypertriton. Evento N. "<<i<<endl;
                Double_t Tbeam = EKin[j]; // Ekin
                Mass[j] = 0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j];
                Double_t Ebeam = EKin[j] + Mass[j]; // Ekin+Mass
                Double_t Pbeam_abs = sqrt(EKin[j] * (EKin[j] + 2.0 * Mass[j]));
                // cout<<"Theoretical mass Hypertriton=2992.14 MeV . With the Formula M= "<<Mass[j]<<" MeV"<<endl;
                double beta = Pbeam_abs / Ebeam;
                double Gamma = 1 / sqrt(1 - (beta * beta));
                // cout<<"Pbeam= "<<Pbeam_abs<<"	Ebeam= "<<Ebeam<<"	beta=	"<<beta<<"	Gamma=	"<<Gamma<<endl;
                TLorentzVector PHyper(px[j], py[j], pz[j], Ebeam);
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
                    TBeamSimData particle(Zdecay[idecay], Adecay[idecay], Pdecay[idecay], pos_decay);
                    if (Zdecay[idecay] == -1)
                    {
                        particle.fParticleName = "pi-";
                        particle.fPDGCode = -211;
                    }
                    if (Zdecay[idecay] == 2 && Adecay[idecay] == 3)
                    {
                        particle.fParticleName = "He3";
                        particle.fPDGCode = 2003;
                    }
                    particle.fPrimaryParticleID = i;
                    gBeamSimDataArray->push_back(particle);
                }
            }
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
            if (PDGCode[j] == 3122)
            {
                // cout<<"You found a free Lambda. Evento N. "<<i<<endl;
                hyper_flag = 1;
                TGenPhaseSpace Decay;
                Int_t Zdecay[Ndecay] = { 1, -1 };
                Int_t Adecay[Ndecay] = { 1, 0 };
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
                    TBeamSimData particle(Zdecay[idecay], Adecay[idecay], Pdecay[idecay], pos_decay);
                    if (Zdecay[idecay] == -1)
                    {
                        particle.fParticleName = "pi-";
                        particle.fPDGCode = -211;
                    }
                    if (Zdecay[idecay] == 1 && Adecay[idecay] == 0)
                    {
                        particle.fParticleName = "proton";
                        particle.fPDGCode = 2212;
                    }
                    particle.fPrimaryParticleID = i;
                    gBeamSimDataArray->push_back(particle);
                }
            }
            //-------------------------------------------------------------------------------------------------------------------------------------------------------
        }
        tree->Fill();
    }
    // write and close everything
    tree->Write();
    file->Close();

    fopen->Close();
}

// Here, at the end of the code I will define the code of the function, to make the code more readable

if (PDGCode[j] == 1010010030)
{

    Int_t Zdecay[Ndecay] = { 2, -1 };
    Int_t Adecay[Ndecay] = { 3, 0 };
    // cout<<"You found an Hypertriton. Event N. "<<i<<endl;
    Double_t Tbeam = EKin[j]; // Ekin
    Mass[j] = 0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j];
    Double_t Ebeam = EKin[j] + Mass[j]; // Ekin+Mass
    Double_t Pbeam_abs = sqrt(EKin[j] * (EKin[j] + 2.0 * Mass[j]));
    // cout<<"Theoretical mass Hypertriton=2992.14 MeV . With the Formula M= "<<Mass[j]<<" MeV"<<endl;
    double beta = Pbeam_abs / Ebeam;
    double Gamma = 1 / sqrt(1 - (beta * beta));
    // cout<<"Pbeam= "<<Pbeam_abs<<"	Ebeam= "<<Ebeam<<"	beta=	"<<beta<<"	Gamma=	"<<Gamma<<endl;
    TLorentzVector PHyper(px[j], py[j], pz[j], Ebeam);

    // Flight decay vector generator
    TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tauHyper);
    f1->SetParameter(0, Gamma * tauHyper);
    Double_t t = f1->GetRandom();
    Double_t l = (Gamma * beta * c * t);
    TVector3 d_length(0, 0, l);
    pos_decay += d_length;

    // Questo va comunque nel main file
    TGenPhaseSpace Decay;
    Decay.SetDecay(PHyper, Ndecay, MassHyper);

    // Generate decayed events
    double weight = Decay.Generate();
    while (gRandom->Uniform() > weight)
        weight = Decay.Generate();

    // The Hypertriton decay
    TLorentzVector Pdecay[Ndecay];
    for (int idecay = 0; idecay < Ndecay; ++idecay)
    {
        Pdecay[idecay] = *(Decay.GetDecay(idecay));
        TBeamSimData particle(Zdecay[idecay], Adecay[idecay], Pdecay[idecay], pos_decay);
        if (Zdecay[idecay] == -1)
        {
            particle.fParticleName = "pi-";
            particle.fPDGCode = -211;
        }
        if (Zdecay[idecay] == 2 && Adecay[idecay] == 3)
        {
            particle.fParticleName = "He3";
            particle.fPDGCode = 2003;
        }
        particle.fPrimaryParticleID = i;
        gBeamSimDataArray->push_back(particle);
    }
}
