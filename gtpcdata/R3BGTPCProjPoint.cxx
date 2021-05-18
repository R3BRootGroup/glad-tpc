/******************************************************************************
 *   Copyright (C) 2020 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2020 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/
#include "R3BGTPCProjPoint.h"
#include <iostream>

R3BGTPCProjPoint::R3BGTPCProjPoint()
{
    fVirtualPadID = 0;
    fCharge = 0.;
    fTimeDistr = NULL;
    fPDGCode = 0;
    fMotherId = 0;
    fx0 = 0;
    fy0 = 0;
    fz0 = 0;
    fpx0 = 0;
    fpy0 = 0;
    fpz0 = 0;
}

R3BGTPCProjPoint::R3BGTPCProjPoint(Int_t pad,
                                   Double_t time,
                                   Double_t charge,
                                   Int_t eventID,
                                   Int_t PdgCode,
                                   Int_t MotherId,
                                   Double_t x0,
                                   Double_t y0,
                                   Double_t z0,
                                   Double_t px0,
                                   Double_t py0,
                                   Double_t pz0)
{
    fVirtualPadID = pad;
    fCharge = charge;
    char hname[255];
    sprintf(hname, "event %i: pad %i", eventID, fVirtualPadID);
    fTimeDistr = new TH1S(hname, hname, 400, 0, 40); // this is for microsec
    // fTimeDistr = new TH1S(hname, hname, 400000, 0, 40000);//this is for nanosec
    SetTimeDistr(time, charge);
    fPDGCode = PdgCode;
    fMotherId = MotherId;
    fx0 = x0;
    fy0 = y0;
    fz0 = z0;
    fpx0 = px0;
    fpy0 = py0;
    fpz0 = pz0;
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
