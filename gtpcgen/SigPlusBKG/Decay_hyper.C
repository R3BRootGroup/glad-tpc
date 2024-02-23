
using namespace std;

#include "./Variables_decay.C"
#include <fstream>
#include <iostream>
#include <tuple>
// Functions declaration
int Decay3LH(double BRatio, int A[], int Z[], double Masses[], int PDGCode[]);
int Decay4LH(double BRatio, int A[], int Z[], double Masses[], int PDGCode[]);
int DecayLambda(double BRatio, int A[], int Z[], double Masses[], int PDGCode[]);
double flight_length(double tau, double beta);
int DecayHyper(double BRatio, int A[], int Z[], double Masses[], int PDGCode[], int ZZ, int AA);
int DecayK0Short(double BRatio, int A[], int Z[], double Masses[], int PDGCode[]);
// Functions definition
int Decay3LH(double BRatio, int A[], int Z[], double Masses[], int PDGCode[])
{
    int branch, multiplicity;
    if (BRatio > 0 && BRatio <= 0.384) // 2 bodies decay
    {
        multiplicity = 2;
        double charge = gRandom->Uniform(0.0000, 1.000);
        if (charge > 0 && charge < 0.666)
            branch = 1; // Pi- He3
        else
            branch = 2; // Pi0 H3
    }
    if (BRatio > 0.384 && BRatio <= 1.003) // 3 bodies decay
    {
        multiplicity = 3;
        double charge = gRandom->Uniform(0.0000, 1.000);
        if (charge > 0 && charge < 0.666)
            branch = 3; // Pi- p deuteron
        else
            branch = 4; // Pi0 n deuteron
    }
    if (BRatio > 1.003 && BRatio <= 1.0127) // 4 bodies decay
    {
        multiplicity = 4;
        double charge = gRandom->Uniform(0.0000, 1.000);
        if (charge > 0 && charge < 0.666)
            branch = 5; // Pi- p p n
        else
            branch = 6; // Pi0 p n n
    }
    // decay->Fill(branch);
    switch (branch)
    {
        case 1: // Pi- He3
            A[0] = 3;
            Z[0] = 2;
            Masses[0] = mass_He3;
            PDGCode[0] = PDGCode_He3; // He3
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            break;

        case 2: // Pi0 H3
            A[0] = 3;
            Z[0] = 1;
            Masses[0] = mass_triton;
            PDGCode[0] = PDGCode_triton; // H3
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            break;

        case 3: // d p Pi-
            A[0] = 2;
            Z[0] = 1;
            Masses[0] = mass_deuteron;
            PDGCode[0] = PDGCode_deuteron; // deuteron
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            A[2] = 1;
            Z[2] = 1;
            Masses[2] = mass_proton;
            PDGCode[2] = PDGCode_proton; // proton
            break;

        case 4: // d n Pi0
            A[0] = 2;
            Z[0] = 1;
            Masses[0] = mass_deuteron;
            PDGCode[0] = PDGCode_deuteron; // deuteron
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            A[2] = 1;
            Z[2] = 0;
            Masses[2] = mass_neutron;
            PDGCode[2] = PDGCode_neutron; // neutron
            break;

        case 5: // p pi- p n
            A[0] = 1;
            Z[0] = 1;
            Masses[0] = mass_proton;
            PDGCode[0] = PDGCode_proton; // proton
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            A[2] = 1;
            Z[2] = 1;
            Masses[2] = mass_proton;
            PDGCode[2] = PDGCode_proton; // proton
            A[3] = 1;
            Z[3] = 0;
            Masses[3] = mass_neutron;
            PDGCode[3] = PDGCode_neutron; // neutron
            break;

        case 6: // p n n pi0
            A[0] = 1;
            Z[0] = 1;
            Masses[0] = mass_proton;
            PDGCode[0] = PDGCode_proton; // proton
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            A[2] = 1;
            Z[2] = 0;
            Masses[2] = mass_neutron;
            PDGCode[2] = PDGCode_neutron; // neutron
            A[3] = 1;
            Z[3] = 0;
            Masses[3] = mass_neutron;
            PDGCode[3] = PDGCode_neutron; // neutron
            break;
    }
    return multiplicity;
}
//-----------------------K0_Short------------------------------------------------------
int DecayK0Short(double BRatio, int A[], int Z[], double Masses[], int PDGCode[])
{
    int branch, multiplicity = 2;
    if (BRatio > 0 && BRatio <= 0.692) // 2 bodies decay
        branch = 1;                    // Pi- Pi+
    else
        branch = 2; // Pi0 Pi0
    switch (branch)
    {
        case 1: // Pi-  Pi+
            A[0] = -1;
            Z[0] = 0;
            Masses[0] = mass_pion_m;
            PDGCode[0] = -PDGCode_pion_m; // pi+
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            break;

        case 2: // Pi0 Pi0
            A[0] = -1;
            Z[0] = 0;
            Masses[0] = mass_pion_0;
            PDGCode[0] = PDGCode_pion_0; // neutron
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            break;
    }
    return multiplicity;
}
//-----------------------LAMBDA--------------------------------------------------------
int DecayLambda(double BRatio, int A[], int Z[], double Masses[], int PDGCode[])
{
    // cout<<"Branching ratio 3 and 4 body less than 3 percentage."<<endl;
    int branch, multiplicity = 2;
    // cout << "Random number generated: " <<BRatio <<endl<<endl;
    if (BRatio > 0 && BRatio <= 0.64) // 2 bodies decay
        branch = 1;                   // Pi- proton
    else
        branch = 2; // Pi0 neutron
    switch (branch)
    {
        case 1: // Pi- proton
            A[0] = 1;
            Z[0] = 1;
            Masses[0] = mass_proton;
            PDGCode[0] = PDGCode_proton; // proton
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            break;

        case 2: // Pi0 neutron
            A[0] = 1;
            Z[0] = 0;
            Masses[0] = mass_neutron;
            PDGCode[0] = PDGCode_neutron; // neutron
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            break;
    }
    return multiplicity;
}
//-----------------------4LH--------------------------------------------------------
int Decay4LH(double BRatio, int A[], int Z[], double Masses[], int PDGCode[])
{
    // cout<<"Branching ratio 3 and 4 body less than 3 percentage."<<endl;
    int branch, multiplicity = 2;
    if (BRatio > 0 && BRatio <= 0.713) // 2 bodies decay
        branch = 1;                    // Pi- He4
    else
        branch = 2; // Pi0 H4
    switch (branch)
    {
        case 1: // Pi- He4
            A[0] = 4;
            Z[0] = 2;
            Masses[0] = mass_He4;
            PDGCode[0] = PDGCode_He4; // He4
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            break;

        case 2: // Pi0 H4
            A[0] = 1;
            Z[0] = 0;
            Masses[0] = mass_H4;
            PDGCode[0] = PDGCode_H4; // H4
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            break;
    }
    return multiplicity;
}
//-----------------------OTHERS--------------------------------------------------------
int DecayHyper(double BRatio, int A[], int Z[], double Masses[], int PDGCode[], int ZZ, int AA)
{
    int branch, multiplicity = 2;
    if (BRatio > 0 && BRatio <= 0.3) // 2 bodies decay
    {
        branch = 1; // Pi- Z+1
        ZZ++;
    }
    else
    {
        branch = 2; // Pi0 Z
    }
    int PDG = 1000000000 + (10000 * ZZ) + (10 * AA);
    double mass = 0;
    std::fstream mass_file("./SigPlusBKG/Z_A_MASS.data", std::ios_base::in);
    if (mass_file.is_open())
    { /*do nothing!*/
    }
    else
    {
        cout << "mass_file cannot be opened!" << endl;
        exit(1);
    }
    while (!mass_file.eof())
    {
        int a, b;
        double c;
        mass_file >> a >> b >> c;
        if (a == ZZ && b == AA)
            mass = c * 0.001; // [GeV/c^2]
    }
    switch (branch)
    {
        case 1: // Pi- Z+1
            if ((AA != 1 && ZZ != 1) || (AA != 1 && ZZ != 0))
            {
                A[0] = AA;
                Z[0] = ZZ;
                Masses[0] = mass;
                PDGCode[0] = PDG; // Z+1
            }
            else if (AA == 1 && ZZ == 1)
            {
                A[0] = AA;
                Z[0] = ZZ;
                Masses[0] = mass;
                PDGCode[0] = 2212;
            }
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_m;
            PDGCode[1] = PDGCode_pion_m; // pi-
            break;

        case 2: // Pi0 Z
            if ((AA != 1 && ZZ != 1) || (AA != 1 && ZZ != 0))
            {
                A[0] = AA;
                Z[0] = ZZ;
                Masses[0] = mass;
                PDGCode[0] = PDG; // Z+1
            }
            else if (AA == 1 && ZZ == 1)
            {
                A[0] = AA;
                Z[0] = ZZ;
                Masses[0] = mass;
                PDGCode[0] = 2212;
            }
            else if (AA == 1 && ZZ == 0)
            {
                A[0] = AA;
                Z[0] = ZZ;
                Masses[0] = mass;
                PDGCode[0] = 2112;
            }
            A[1] = -1;
            Z[1] = 0;
            Masses[1] = mass_pion_0;
            PDGCode[1] = PDGCode_pion_0; // pi0
            break;
    }

    return multiplicity;
}
//-----------------------DECAY
// LENGTH--------------------------------------------------
double flight_length(double beta, double tau)
{
    double Gamma = 1 / sqrt(1 - (beta * beta));
    TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tau);
    f1->SetParameter(0, Gamma * tau);
    Double_t t = f1->GetRandom();
    Double_t l = (beta * c * t);
    return l;
}

// particle progate form O1 to O2
void flight_in_B(TLorentzVector P1, TVector3 O1, double C1, double tau, TVector3& O2)
{
    O2.Clear();
    double Gamma = P1.Gamma();
    TF1* f1 = new TF1("f1", "exp(-x/[0])", 0, 11 * tau);
    f1->SetParameter(0, Gamma * tau);
    Double_t t1e = f1->GetRandom(); // when stop
    double B = 2.2;
    TVector3 vB(0, B, 0);
    double mass_proton = 938.27200; // MeV
    double clight = 299.792458;     // mm/nsec

    double m1 = P1.M();
    double omegap = 9.58E7; // Larmor frequency of proton
    double omega1 = C1 * B * mass_proton / m1 * omegap;
    omega1 = omega1 * 1E-9; // s^-1-->ns^-1

    double v1 = P1.Beta() * clight;
    double v1x = v1 * P1.Px() / P1.P();
    double v1y = v1 * P1.Py() / P1.P();
    double v1z = v1 * P1.Pz() / P1.P();
    double x1 = O1.X();
    double y1 = O1.Y();
    double z1 = O1.Z();
    double vv1 = TMath::Sqrt(v1x * v1x + v1z * v1z);
    // initial phase
    double phase1 = TMath::ATan(-v1x / v1z);
    // double phase1 = TMath::ASin(-v1x/TMath::Sqrt(v1x*v1x + v1z*v1z));

    if (C1 == 0)
    {
        double x1_tmp = x1 + t1e * v1x;
        double z1_tmp = z1 + t1e * v1z;
        double y1_tmp = y1 + t1e * v1y;
        O2.SetXYZ(x1_tmp, y1_tmp, z1_tmp);
    }
    else
    {
        double r1 = vv1 / omega1;

        double x1_tmp = x1 + r1 * (cos(omega1 * t1e + phase1) - cos(phase1));
        double z1_tmp = z1 + r1 * (sin(omega1 * t1e + phase1) - sin(phase1));
        double y1_tmp = y1 + v1y * t1e;
        O2.SetXYZ(x1_tmp, y1_tmp, z1_tmp);
    }
}
