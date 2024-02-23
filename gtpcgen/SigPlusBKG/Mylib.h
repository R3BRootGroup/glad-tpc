/*AUTHOR: SIMONE VELARDITA
Creation date:07-Apr-2022
Backup:

This macro has the aim to collect all the variables and functions that I usually
use. This is my personal library!
*/
//--------------Libraries declaration------------------------------------------
// C++
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <string>
#include <unistd.h> //timing
#include <vector>

// ROOT
#include <Riostream.h>
#include <TArrow.h>
#include <TBranch.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TGenPhaseSpace.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TPad.h>
#include <TProfile.h>
#include <TRandom3.h>
#include <TString.h>
#include <TTree.h>
#include <TVector3.h>

using namespace std;

// ALL THE VARIABLES NEED TO BE IN [CM], [SEC], [GeV]...

//--------------Variables declaration------------------------------------------
// Conversions constant
const double fermi = pow(10, -13);    // [cm]
const double barn = pow(10, -24);     // [cm^2]
const double mbarn = pow(10, -27);    // [cm^2]
const double pico_sec = pow(10, -12); // [s]
const double day = 86400;             // [s]
const double atomic_mass = 0.93150;   // GeV/c^2 unified atomic mass unit

// Physics constant
const double N_a = 6.022 * pow(10, 23);

// 3LH decay: lifetime= 217 ps-> world average C. Rappold et al. Physics Letters
const double c = 29.9792; // speed of light cm/ns

//--------------Functions declaration------------------------------------------
// Loading bar_________________________________________________________________
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void loadfunction(double percentage);

// Loading bar_________________________________________________________________
void loadfunction(double percentage)
{
    int val = (int)(percentage * 100);
    int lpad = (int)(percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}
