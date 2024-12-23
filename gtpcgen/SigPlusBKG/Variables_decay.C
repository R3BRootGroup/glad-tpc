#include <iostream>
#include <string>
// #include <filesystem>
#include <unistd.h>
// decayed products
double mass_pion_m = 0.13957;
int PDGCode_pion_m = -211;
double mass_pion_0 = 0.1349;
int PDGCode_pion_0 = 111;
double mass_neutron = 0.93957;
int PDGCode_neutron = 2112;
double mass_proton = 0.938272;
int PDGCode_proton = 2212;
double mass_deuteron = 1.87612;
int PDGCode_deuteron = 1000010020;
double mass_triton = 2.80943;
int PDGCode_triton = 1000010030;
double mass_H4 = 4.02643;
int PDGCode_H4 = 1000010040;
double mass_He3 = 2.80839;
int PDGCode_He3 = 1000020030;
double mass_He4 = 3.727379;
int PDGCode_He4 = 1000020040;
double mass_k0short = 0.497614;
int PDGCode_k0short = 130;

// Hypernuclei parameters

double tau_3LH = 0.227; // ns
double tau_4LH = 0.208;
double tau_k0short = 0.08954;
double tau_lambda = 0.263;

void Variables()
{
    char tmp[256];
    getcwd(tmp, 256);
    std::cout << "Variables to be used for the decay are defined in :\n" << tmp << "/Variables_decay.C" << std::endl;
}
