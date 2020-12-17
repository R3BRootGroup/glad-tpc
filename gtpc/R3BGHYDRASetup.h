#ifndef R3BGHYDRASETUP_H
#define R3BGHYDRASETUP_H 1

#include <cstdlib>
#include <fstream>
#include <iostream>
#include "TObject.h"
#include "TRandom.h"
#include "TMath.h"
#include <vector>

using namespace std;

class R3BGHYDRASetup:public TObject {
  
 public:
  
  R3BGHYDRASetup();
  R3BGHYDRASetup(int verbose);
  ~R3BGHYDRASetup();
  void ReadConfigurationFile(string,int);
  inline TString GetOutputFileName() {return OutputFileName;}

  //Defined by Simone----------------------------------------------------------

  inline double GetActiveRegionx() {return ActiveRegionx;}
  inline double GetActiveRegiony() {return ActiveRegiony;}
  inline double GetActiveRegionz() {return ActiveRegionz;}
  inline double GetTPCLx() {return TPCLx;}
  inline double GetTPCLy() {return TPCLy;}
  inline double GetTPCLz() {return TPCLz;}
  inline double GetFrameThickness() {return FrameThickness;}
  inline double GetWindowx() {return Windowx;}
  inline double GetWindowy() {return Windowy;}
  inline double GetWindowz() {return Windowz;}

  inline double GetTargetLength() {return TargetLength;}
  inline double GetTargetRadius() {return TargetRadius;}  
  inline double GetTargetAngle() {return TargetAngle;}
  inline double GetTargetOffsetX() {return TargetOffsetX;}
  inline double GetTargetOffsetY() {return TargetOffsetY;}
  inline double GetTargetOffsetZ() {return TargetOffsetZ;}
  
  inline double GetFieldAngle() {return FieldAngle;}
  inline double GetFieldOffsetX() {return FieldOffsetX;}
  inline double GetFieldOffsetY() {return FieldOffsetY;}
  inline double GetFieldOffsetZ() {return FieldOffsetZ;}
  
  inline double GetGladAngle() {return GladAngle;}
  inline double GetGladOffsetX() {return GladOffsetX;}
  inline double GetGladOffsetY() {return GladOffsetY;}
  inline double GetGladOffsetZ() {return GladOffsetZ;}
  
  void Print();
  
 protected:
  
  //initial conditions

  double ActiveRegionx, ActiveRegiony, ActiveRegionz;
  double TPCLx,TPCLy, TPCLz, FrameThickness;
	double Windowx, Windowy, Windowz;
  double TargetRadius, TargetLength, TargetAngle, TargetOffsetX, TargetOffsetY, TargetOffsetZ;
  double GladAngle, GladOffsetX, GladOffsetY, GladOffsetZ;
  double FieldAngle, FieldOffsetX, FieldOffsetY, FieldOffsetZ;
  TString OutputFileName;
  
  ClassDef(R3BGHYDRASetup,1)
};

#endif
