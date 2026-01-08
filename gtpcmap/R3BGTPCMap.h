/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#pragma once

#include <boost/multi_array.hpp>

#include "TCanvas.h"
#include "TDOMParser.h"
#include "TFile.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "TObject.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TXMLNode.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

class R3BGTPCMap : public TObject
{

  public:
    R3BGTPCMap();
    ~R3BGTPCMap();

    typedef boost::multi_array<double, 3> multiarray;
    typedef multiarray::index index;

    void GeneratePadPlane();
    Int_t BinToPad(Int_t binval);
    std::vector<Float_t> CalcPadCenter(Int_t PadRef);
    TH2Poly* GetPadPlane();

  private:
    multiarray fPadCoord;
    multiarray* fPadCoordPtr;
    std::map<std::vector<int>, int> fPadMap;
    TH2Poly* fPadPlane;

    ClassDefOverride(R3BGTPCMap, 1);
};
