/*
AUTHOR: SIMONE VELARDITA
 Creation date:26-Apr-2022
 Backup:none
HOW TO USE:
 $root run_gen.C
*/
#include "./SigPlusBKG/Decay_hyper.C"
#include "./SigPlusBKG/Mylib.h"
// Functions declaration
void SigplusBkg_generator();
tuple<double, double, double> target();
// For the particle position
TRandom3 r3(0);
const double GeV = 0.001;
void SigplusBkg_generator()
{
    std::string GEOTAG = "Prototype";
    // Open the NHL file
    TFile* fopen = new TFile("./root/input_beam_dcm_C12C12_1.9_all.root");
    if (fopen->IsOpen())
        cout << "File correctly opened!" << endl;
    else
        exit(1);
    // output file
    ofstream asciifile;

    cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
    TString filename = "./ASCII/input" + GEOTAG + "_SignalBKG_Kaons.dat";
    asciifile.open(filename, ios::out);

    TTree* t = (TTree*)fopen->Get("tree_input");
    Int_t nmax = 40;    // max dimension of the array
    Short_t nParticles; // number of particles after the collision
    // characteristics of the produced particles after the collision
    Short_t A[nmax], Z[nmax], S[nmax];
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

    // sETUP INFORMATION TODO use parameters
    double TargetX = -2.7; // curvature beam  [cm]
    double TargetY = 0;
    double TargetZ = 227;

    // Create the Root file and tree that will be used by the simulation
    Short_t multiplicity; // number of particles after the collision and the decay
    vector<int> AA,
        ZZ; // characteristic of the produced particles after the collision
    vector<float> pxx, pyy, pzz, MASS, xx, yy, zz;
    vector<int> PDG, EventID;

    //-------------------------------------------------------------------------------------------------------------------------------------------------------
    // Now we have to loop inside the NHL file
    Int_t Nentr = (Int_t)t->GetEntries();
    int counter_skip = 0;
    cout << "Total number of entries inside " << Nentr << endl << endl;
    for (int i = 0; i < Nentr; ++i) // Nentr
    {
        // Clear all the vector I'll use later
        MASS.clear();
        pxx.clear();
        pyy.clear();
        pzz.clear();
        xx.clear();
        yy.clear();
        zz.clear();
        PDG.clear();
        EventID.clear();
        AA.clear();
        ZZ.clear();

        double percentage = i / (Nentr * 1.0);
        loadfunction(percentage);
        // Position
        TVector3 pos_vec, pos_decay;
        TVector3 pos_decay2;
        auto [tgtx, tgty, tgtz] = target(); // tuple
        pos_vec.SetXYZ(tgtx + TargetX, tgty + TargetY, tgtz + TargetZ);
        pos_decay = pos_vec;

        // Now for each event we have to loop on the number of particles produced
        // after the collision
        t->GetEntry(i);
        int Nsize = nParticles;
        multiplicity = nParticles;
        int counter_piHe = 0;
        float Mass[Nsize];
        TLorentzVector P[Nsize];
        for (int j = 0; j < Nsize; ++j)
        {
            if ((PDGCode[j] == -211 || PDGCode[j] == 1000020030) && S[j] != -1 && !isnan(px[j]))
            {
                Mass[j] = (0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j]) * GeV;
                MASS.push_back(Mass[j]);
                pxx.push_back(px[j] * GeV);
                pyy.push_back(py[j] * GeV);
                pzz.push_back(pz[j] * GeV);
                xx.push_back(pos_vec.x());
                yy.push_back(pos_vec.y());
                zz.push_back(pos_vec.z());
                PDG.push_back(PDGCode[j]);
                EventID.push_back(i);
                AA.push_back(A[j]);
                ZZ.push_back(Z[j]);
                counter_piHe++;
            }
            if (PDGCode[j] == 130 || PDGCode[j] == 3122 ||
                PDGCode[j] > 1010000030) // hypernucleusPDGCode[j]==1010010030
            {
                int Adecay[4], Zdecay[4], PDGCode_decay[4];
                double MassHyper[4];
                int Ndecay = 0;
                double l = 0;
                TGenPhaseSpace Decay;
                double BRatio = gRandom->Uniform(0.000, 1.000);
                Mass[j] = (0.5 * (px[j] * px[j] + py[j] * py[j] + pz[j] * pz[j] - EKin[j] * EKin[j]) / EKin[j]);
                TLorentzVector PHyper(px[j] * GeV,
                                      py[j] * GeV,
                                      pz[j] * GeV,
                                      EKin[j] * GeV + Mass[j] * GeV); // GeV for PhaseSpace

                TLorentzVector Pm;
                Pm.SetPxPyPzE(px[j], py[j], pz[j], Mass[j] + EKin[j]); // MeV

                // Hypertriton
                if (PDGCode[j] == 1010010030)
                {
                    Ndecay = Decay3LH(BRatio, Adecay, Zdecay, MassHyper, PDGCode_decay);
                    flight_in_B(Pm, pos_decay * 10, 1., tau_3LH,
                                pos_decay2); // mm, ns and MeV
                }
                // Free_Lambda
                if (PDGCode[j] == 3122)
                {
                    Ndecay = DecayLambda(BRatio, Adecay, Zdecay, MassHyper, PDGCode_decay);
                    flight_in_B(Pm, pos_decay * 10., 0., tau_lambda,
                                pos_decay2); // mm, ns and MeV
                }
                if (PDGCode[j] == 130)
                {
                    if (BRatio < 0.5) // only half of the k0 are k0short
                    {
                        double BRatio_k = gRandom->Uniform(0.000, 1.000);
                        Ndecay = DecayK0Short(BRatio_k, Adecay, Zdecay, MassHyper, PDGCode_decay);
                        flight_in_B(Pm, pos_decay * 10., 0., tau_k0short,
                                    pos_decay2); // mm, ns and MeV
                    }
                    else
                        continue;
                }
                else if (PDGCode[j] != 1010010030 && PDGCode[j] != 3122 && PDGCode[j] != 130)
                {
                    Ndecay = DecayHyper(BRatio, Adecay, Zdecay, MassHyper, PDGCode_decay, Z[j], A[j]);
                    flight_in_B(Pm, pos_decay * 10., Z[j], tau_4LH,
                                pos_decay2); // mm, ns and MeV
                }
                multiplicity += Ndecay - 1;
                pos_decay = pos_decay2 * 0.1; // cm

                Decay.SetDecay(PHyper, Ndecay, MassHyper);

                // Generate decayed events
                double weight = Decay.Generate();
                // The Hypernuclei decay products
                TLorentzVector Pdecay[Ndecay];

                for (int idecay = 0; idecay < Ndecay; ++idecay)
                {
                    if (PDGCode_decay[idecay] == 1000020030 || PDGCode_decay[idecay] == -211)
                    {
                        Pdecay[idecay] = *(Decay.GetDecay(idecay));
                        MASS.push_back(MassHyper[idecay]);
                        xx.push_back(pos_decay.x());
                        yy.push_back(pos_decay.y());
                        zz.push_back(pos_decay.z());
                        pxx.push_back((Pdecay[idecay].Px()));
                        pyy.push_back((Pdecay[idecay].Py()));
                        pzz.push_back((Pdecay[idecay].Pz()));
                        PDG.push_back(PDGCode_decay[idecay]);
                        ZZ.push_back(Zdecay[idecay]);
                        AA.push_back(Adecay[idecay]);
                        EventID.push_back(i);
                        counter_piHe++;
                    }
                }
            }
        }
        if (counter_piHe < 2)
        {
            counter_skip++;
            continue;
        }
        else
        {
            if (asciifile.is_open())
            {
                asciifile << i - counter_skip << "  " << counter_piHe << "  0.  0."
                          << "\n"; // multiplicity
            }
            for (int jj = 0; jj < counter_piHe; ++jj) // multiplicity
            {
                /*cout<<"PDGCode="<<PDG.at(jj)<<"	ZZ="<< ZZ.at(jj) <<"  AA="<<
                   AA.at(jj)<<"  " << pxx.at(jj) << "  " << pyy.at(jj) << "  " <<
                   pzz.at(jj)
                              << "  " << xx.at(jj) << "  " << yy.at(jj) << "  " <<
                   zz.at(jj) << "  " << MASS.at(jj)
                              << "\n";*/
                if (asciifile.is_open())
                {
                    if (PDG.at(jj) == -211)
                    {
                        asciifile << PDG.at(jj) << "	0  " << PDG.at(jj) << "  " << pxx.at(jj) << "  " << pyy.at(jj)
                                  << "  " << pzz.at(jj) << "  " << xx.at(jj) << "  " << yy.at(jj) << "  " << zz.at(jj)
                                  << "  " << MASS.at(jj) << "\n";
                    }
                    if (PDG.at(jj) == 1000020030)
                    {
                        asciifile << "-1 " << ZZ.at(jj) << "  " << AA.at(jj) << "  " << pxx.at(jj) << "  " << pyy.at(jj)
                                  << "  " << pzz.at(jj) << "  " << xx.at(jj) << "  " << yy.at(jj) << "  " << zz.at(jj)
                                  << "  " << MASS.at(jj) << "\n";
                    }
                }
            }
        }
    }
    // DUMMY HYPERTRITON
    // DECAY-------------------------------------------------------------------------------------------------------------------------------
    {
        // Decayed particles characteristics
        TRandom3 r3(0), E(0);
        const Int_t AHyper = 3;
        const Int_t Afrag = 3;
        const Int_t Zfrag = 2;
        const Int_t Ndecay = 2;
        const Int_t Z[Ndecay] = { Zfrag, -1 };
        const Int_t A[Ndecay] = { Afrag, 0 };
        const Double_t Masses[Ndecay] = { 2.80839, 0.13957 }; // 3He,pi

        auto [tgtx, tgty, tgtz] = target(); // tuple
        // Init Phase Space Decay
        double MHyper = 2.99214; // 3LH -> D.H.Davis Nucl. Phys. A 754 (2006) 3
        TGenPhaseSpace Decay;
        //____________________________________________________
        for (int i = 0; i < 10000; ++i)
        {
            // Hypertriton Ekinetic
            double sig_E = 0.1344, mean_E = 1.555; //[GeV]
            double E_in = E.Gaus(mean_E, sig_E);

            Double_t THyper = AHyper * E_in;                              //[GeV]
            Double_t EHyper = MHyper + THyper;                            //[GeV]
            Double_t PHyper_abs = sqrt(THyper * (THyper + 2.0 * MHyper)); //[GeV]

            TLorentzVector PHyper(0, 0, PHyper_abs, EHyper); //[GeV]
            // angular spread[rad], deduced from Dubna cascade model+fermi break-up
            Double_t xang = gRandom->Landau(0.016, 0.006); // theta
            Double_t yang = gRandom->Landau(-0.29, 0.8);   // phi
            PHyper.SetTheta(xang);
            PHyper.SetPhi(yang);

            TVector3 pos_vec;
            auto [tgtx, tgty, tgtz] = target();                  // tuple
            pos_vec.SetXYZ(tgtx + (-2.7), tgty + 0, tgtz + 227); // cm

            Decay.SetDecay(PHyper, Ndecay, Masses);

            // Generate decayed events
            double weight = Decay.Generate();

            TVector3 pos_decay2;
            flight_in_B(PHyper * 1000, pos_vec * 10, 1., tau_3LH, pos_decay2);
            pos_vec = pos_decay2 * 0.1;

            if (asciifile.is_open())
            {
                asciifile << (i + Nentr) - counter_skip << "  " << 2 << "  0.  0."
                          << "\n"; // multiplicity
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
                    if (Z[j] == -1) // pi-
                    {
                        asciifile << "1  0  -211  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
                                  << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                                  << "  0.13957"
                                  << "\n";
                    }
                    if (Z[j] == 2) // He3
                    {
                        asciifile << "-1  2  3  " << P[j].Px() << "  " << P[j].Py() << "  " << P[j].Pz() << "  "
                                  << pos_vec.x() << "  " << pos_vec.y() << "  " << pos_vec.z() << "  "
                                  << "  2.80839"
                                  << "\n";
                    }
                }
            }
        }
    }
    cout << "total n events that have at least one pion or one He3" << endl;
    cout << Nentr - counter_skip + 10000 << endl;
    cout << endl << endl;
    // write and close everything
    fopen->Close();

    cout << "-----------END OF DECAY FILE------------------------------------" << endl << endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

tuple<double, double, double> target()
{
    double TargetRadius = 0.5, TargetLength = 5.;
    TVector3 TargetPosition(0, 0, 0);
    float FWHM = 0.2; // cm
    float sig_x = FWHM / 2.355, sig_y = FWHM / 2.355;
    float tgtx = -999, tgty = -999;
    while (sqrt(tgtx * tgtx + tgty * tgty) > TargetRadius)
    {
        tgtx = r3.Gaus(0, sig_x);
        tgty = r3.Gaus(0, sig_y);
    }
    double tgtz = r3.Uniform(-0.5 * TargetLength, 0.5 * TargetLength);
    return { tgtx, tgty, tgtz };
}
