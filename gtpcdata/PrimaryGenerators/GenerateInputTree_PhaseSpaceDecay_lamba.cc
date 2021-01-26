// HOW TO USE
// root [0] .L GenerateInputTree_PhaseSpaceDecay.cc+
// root [1] GenerateInputTree_PhaseSpaceDecay()

#include "TBeamSimData.hh"

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

#include <iostream>
using namespace std;

// For flight decay vector
// double tau=0.216;//3LH decay time in ns-> world average C. Rappold et al. Physics Letters B 728 (2014) 543–548
// --->0.263
double tau = 0.263; // Lambda decay time in ns-> 0.263
double c = 299.792; // mm/ns

void GenerateInputTree_PhaseSpaceDecay()
{
    // parameteres
    const int neve = 10000;

    TRandom3 r3(0);
    const Int_t Abeam = 1;
    const Int_t Afrag = 1;
    const Int_t Zfrag = 1;
    const Int_t Ndecay = 2;
    // Double_t E_in=2000;//Beam incident energy MeV/nucleon for Hypertriton decay
    Double_t E_in = 1900; // Beam incident energy MeV/nucleon for lambda decay

    TCanvas* c1 = new TCanvas();
    TCanvas* c2 = new TCanvas();
    TCanvas* c3 = new TCanvas();
    TH1D* length = new TH1D("length", "Decay length; l [mm]; N. events (a.u.)", 110, 0, 2200);
    TH1D* totlength = new TH1D("Tot length", "Total decay length (z); l [mm]; N. events (a.u.)", 230, 1300, 3600);

    TH1D* time = new TH1D("time", "#tau =263 [ps];;N. events (a.u.)", 238, 0, 2.4);
    Int_t n = 11;
    for (int i = 0; i < 11; i++)
    {
        time->GetXaxis()->SetBinLabel(22 * i, Form("%d #tau", i));
    }

    const Int_t Z[Ndecay] = { Zfrag, -1 };
    const Int_t A[Ndecay] = { Afrag, 0 };
    const Double_t Masses[Ndecay] = { 938.272, 139.57 }; // proton pi-

    // Init Phase Space Decay
    double Mbeam = 1115.683; // lambda 1115.683MeV
    Double_t Tbeam = Abeam * E_in;
    Double_t Ebeam = Mbeam + Tbeam;
    Double_t Pbeam_abs = sqrt(Tbeam * (Tbeam + 2.0 * Mbeam));
    TGenPhaseSpace Decay;

    double beta = Pbeam_abs / Ebeam;
    double Gamma = 1 / sqrt(1 - (beta * beta));
    cout << "Pbeam= " << Pbeam_abs << "	Ebeam= " << Ebeam << "	beta=	" << beta << "	Gamma=	" << Gamma << endl;

    TFile* file = new TFile("../root/inputtree_3LH.root", "recreate");
    TTree* tree = new TTree("tree_input", "Input tree for simtrace");
    gBeamSimDataArray = new TBeamSimDataArray();
    tree->Branch("TBeamSimData", gBeamSimDataArray);

    //____________________________________________________
    for (int i = 0; i < neve; ++i)
    {
        gBeamSimDataArray->clear();

        TLorentzVector Pbeam(0, 0, Pbeam_abs, Ebeam);

        //_____________________________________________________everything is  mm (don't modify the units!!!)
        double TargetRadius = 2.5, TargetLength = 50.; // before radius was 25.
        TVector3 TargetPosition(-5.25, 0., 1825.);     // for hydra 1425
        double FWHM = 4;
        double sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
        // cout<<"sigma x= "<<sig_x<<"	sigma y= "<<sig_y<<endl;
        double tgtx = -999, tgty = -999;
        while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius)
        {
            tgtx = r3.Gaus(0, sig_x);
            tgty = r3.Gaus(0, sig_y);
        }
        double tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
        double angle = -2.25; // deg, rotate angle in lab frame
        TVector3 pos_vec;
        pos_vec.SetXYZ(tgtx, tgty, tgtz);
        pos_vec += TargetPosition;

        // angular spread xy: +-10mrad
        Double_t xang = gRandom->Gaus(0, 0.01);
        Double_t yang = gRandom->Gaus(0, 0.01);
        Pbeam.RotateY(xang);
        Pbeam.RotateX(-yang);

        Decay.SetDecay(Pbeam, Ndecay, Masses);

        // Generate decayed events
        double weight = Decay.Generate();
        while (gRandom->Uniform() > weight)
            weight = Decay.Generate();

        // Flight decay vector generator
        TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tau);
        f1->SetParameter(0, Gamma * tau);
        Double_t t = f1->GetRandom();
        time->Fill(t);
        Double_t l = (Gamma * beta * c * t);
        // length->Fill(l);

        TVector3 d_length(0, 0, l);
        pos_vec += d_length;
        length->Fill(d_length.Z());
        totlength->Fill(pos_vec.Z());

        TLorentzVector P[Ndecay];
        for (int i = 0; i < Ndecay; ++i)
        {
            P[i] = *(Decay.GetDecay(i));
            TBeamSimData particle(Z[i], A[i], P[i], pos_vec);
            if (Z[i] == 0)
                particle.fParticleName = "neutron";
            if (Z[i] == -1)
                particle.fParticleName = "pi-";
            if (Z[i] == 1)
                particle.fParticleName = "proton";
            particle.fPrimaryParticleID = i;
            gBeamSimDataArray->push_back(particle);
        }

        // Carbon12 beam
        double MC = 11200.0;     // 12u->1u=931.50*MeV/c^2
        Double_t TC = 12 * 1900; // 1.9*GeV/A
        Double_t EC = MC + TC;
        Double_t PC_abs = sqrt(TC * (TC + 2.0 * MC));
        TLorentzVector PC(0, 0, PC_abs, EC);
        TBeamSimData particle(6, 12, PC, TVector3(0, 0, -2000)); //-2*m
        particle.fParticleName = "";
        gBeamSimDataArray->push_back(particle);

        // Fill tree
        tree->Fill();

        c1->cd();
        time->Draw();
        c2->cd();
        length->Draw();
        c3->cd();
        totlength->Draw();
    }
    tree->Write();
    file->Close();
}
