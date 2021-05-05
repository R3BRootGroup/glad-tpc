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
/**  R3BGTPCProjPoint.h
 **  A R3BGTPCProjPoint is the projected information on a virtual
 **  pad plane. Contains the total charge and time information
 **  and the virtual pad identifier.
 **/

#ifndef R3BGTPCPROJPOINT_H
#define R3BGTPCPROJPOINT_H

#include "TH1S.h"
#include "TObject.h"

class R3BGTPCProjPoint : public TObject
{
  public:
    /** Default constructor **/
    R3BGTPCProjPoint();

    /** Constructor with data **/
    R3BGTPCProjPoint(Int_t pad, Double_t time, Double_t charge, Int_t eventID);

    /** Destructor **/
    ~R3BGTPCProjPoint();

    /** Accessors **/
    Int_t GetVirtualPadID() const { return fVirtualPadID; }
    Double_t GetCharge() const { return fCharge; }
    TH1S* GetTimeDistribution() const { return fTimeDistr; }
    Int_t GetTimeDistribution(Int_t bin) { return fTimeDistr->GetBinContent(bin); }

    /** Modifiers **/
    void SetVirtualPadID(Int_t pad) { fVirtualPadID = pad; }
    void SetCharge(Double_t cha) { fCharge = cha; }
    void AddCharge() { fCharge = fCharge + 1; }
    void SetTimeDistr(Double_t time, Double_t weight) { fTimeDistr->Fill(time, weight); }

    void Clear(Option_t* option);

  private:
    Int_t fVirtualPadID; //!< Virtual pad Identifier
    Double_t fCharge;    //!< Charge [electrons]
    TH1S* fTimeDistr;    //!< Time distribution in the pad [0.1 microsecond/bin]

    ClassDef(R3BGTPCProjPoint, 1)
};

#endif // R3BGTPCPROJPOINT_H
