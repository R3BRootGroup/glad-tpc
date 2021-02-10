// -------------------------------------------------------------------------
// -----                R3BGTPCProjPoint source file                   -----
// -----               Created  28/03/18  by H. Alvarez Pol            -----
// -------------------------------------------------------------------------

#include "R3BGTPCProjPoint.h"
#include <iostream>

R3BGTPCProjPoint::R3BGTPCProjPoint()
{
    fVirtualPadID = 0;
    fCharge = 0.;
    fTimeDistr = NULL;
}

R3BGTPCProjPoint::R3BGTPCProjPoint(Int_t pad, Double_t time, Double_t charge, Int_t eventID)
{
    fVirtualPadID = pad;
    fCharge = charge;
    char hname[255];
    sprintf(hname, "event %i: pad %i", eventID, fVirtualPadID);
    fTimeDistr = new TH1S(hname, hname, 400, 0, 40);//this is for microsec
    //fTimeDistr = new TH1S(hname, hname, 400000, 0, 40000);//this is for nanosec
    SetTimeDistr(time, charge);

}

R3BGTPCProjPoint::~R3BGTPCProjPoint()
{
    if (fTimeDistr)
        delete fTimeDistr;
}

void R3BGTPCProjPoint::Clear(Option_t* option)
{
    if (fTimeDistr)
        delete fTimeDistr;
}

ClassImp(R3BGTPCProjPoint)
