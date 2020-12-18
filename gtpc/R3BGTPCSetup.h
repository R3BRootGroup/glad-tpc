#ifndef R3BGTPCSETUP_H
#define R3BGTPCSETUP_H 1

#include <cstdlib>
#include <fstream>
#include <iostream>
#include "TObject.h"
#include "TRandom.h"
#include "TMath.h"
#include <vector>

using namespace std;

class R3BGTPCSetup:public TObject {
  
 public:
  
  R3BGTPCSetup();
  R3BGTPCSetup(int verbose);
  ~R3BGTPCSetup();
  void ReadConfigurationFile(string,int);
  inline TString GetOutputFileName() {return OutputFileName;}

  //---------------------------------------------------------------------------

  inline double GetTargetLength() {return TargetLength;}
  inline double GetTargetRadius() {return TargetRadius;}
  inline double GetChamberInnerRadius() {return ChamberInnerRadius;}
  inline double GetChamberThickness() {return ChamberThickness;}

  inline double GetChamberLength() {return ChamberLength;}
  inline double GetInnerRohacellThickness() {return InnerRohacellThickness;}
  inline double GetKaptonThickness() {return KaptonThickness;}
  inline double GetOuterRohacellThickness() {return OuterRohacellThickness;}

  inline double GetTPCRadiusExt() {return TPCRadiusExt;}
  inline double GetWindowThickness() {return WindowThickness;}
  inline double GetTPCLx() {return TPCLx;}
  inline double GetTPCLy() {return TPCLy;}
  inline double GetTPCLz() {return TPCLz;}
  
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

  //Defined by Simone----------------------------------------------------------
  double ActiveRegionx, ActiveRegiony, ActiveRegionz;

  //---------------------------------------------------------------------------

  double TargetRadius, TargetLength, ChamberInnerRadius, ChamberThickness;
  double ChamberLength, InnerRohacellThickness, KaptonThickness, OuterRohacellThickness;
  double TPCRadiusExt, WindowThickness;
  double TPCLx,TPCLy, TPCLz;
  double TargetAngle, TargetOffsetX, TargetOffsetY, TargetOffsetZ;
  double GladAngle, GladOffsetX, GladOffsetY, GladOffsetZ;
  double FieldAngle, FieldOffsetX, FieldOffsetY, FieldOffsetZ;
  TString OutputFileName;
  
  ClassDef(R3BGTPCSetup,1)
};

#endif
