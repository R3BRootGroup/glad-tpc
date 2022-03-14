#ifndef R3BGTPCMAP_H
#define R3BGTPCMAP_H

#include <boost/multi_array.hpp>

#include <cassert>
#include "TObject.h"
#include "TROOT.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include <map>
#include <vector>
#include <set>
#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TFile.h"
#include <fstream>
#include <iostream>

class R3BGTPCMap : public TObject {

 public:
  R3BGTPCMap();
  ~R3BGTPCMap();

  typedef boost::multi_array<double, 3> multiarray;
  typedef multiarray::index index;

  void GeneratePadPlane();                                   
  Int_t BinToPad(Int_t binval);                        
  std::vector<Float_t> CalcPadCenter(Int_t PadRef);
  TH2Poly *GetPadPlane();

 private:

  multiarray fPadCoord;
  multiarray *fPadCoordPtr;
  std::map<std::vector<int>, int> fPadMap;
  TH2Poly *fPadPlane;
  

  ClassDefOverride(R3BGTPCMap, 1);
};

#endif

