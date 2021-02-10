#ifndef R3BGTPCSETUP_H
#define R3BGTPCSETUP_H 1

#include "TMath.h"
#include "TObject.h"
#include "TRandom.h"
#include "TString.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

class R3BGTPCSetup : public TObject
{

  public:
    R3BGTPCSetup();
    R3BGTPCSetup(std::string detector, int verbose);
    ~R3BGTPCSetup();
    void ReadConfigurationFile(string, int);
    inline TString GetOutputFileName() { return OutputFileName; }
    //Electronic______________________________________________________________________
    inline double GetGain() 				{ return Gain; }
    inline double GetTheta() 				{ return Theta; }
    inline double GetNoiseRMS() 		{ return NoiseRMS; }
    inline double GetTimeBinSize() 	{ return TimeBinSize; }
    inline double GetShapingTime() 	{ return ShapingTime; }
    inline double GetThreshold() 		{ return Threshold; }
    //Target__________________________________________________________________________
    inline double GetTargetLength() { return TargetLength; }
    inline double GetTargetRadius() { return TargetRadius; }
    inline double GetTargetAngle() { return TargetAngle; }
    inline double GetTargetOffsetX() { return TargetOffsetX; }
    inline double GetTargetOffsetY() { return TargetOffsetY; }
    inline double GetTargetOffsetZ() { return TargetOffsetZ; }
		//TPC_____________________________________________________________________________
    inline double GetActiveRegionx() { return ActiveRegionx; }
    inline double GetActiveRegiony() { return ActiveRegiony; }
    inline double GetActiveRegionz() { return ActiveRegionz; }
    inline double GetTPCLx() { return TPCLx; }
    inline double GetTPCLy() { return TPCLy; }
    inline double GetTPCLz() { return TPCLz; }
    inline double GetFrameThickness() { return FrameThickness; }
    inline double GetWindowx() { return Windowx; }
    inline double GetWindowy() { return Windowy; }
    inline double GetWindowz() { return Windowz; }
		//Drift___________________________________________________________________________
    inline double GetLongDiff() { return LongDiff; }
    inline double GetTransDiff() { return TransDiff; }
    inline double GetPadSize() { return PadSize; }
    inline double GetFanoFactor() { return FanoFactor; }
    inline double GetEIonization() { return EIonization; }
    inline double GetDriftVelocity() { return DriftVelocity; }
		//GLAD____________________________________________________________________________
    inline double GetFieldAngle() { return FieldAngle; }
    inline double GetFieldOffsetX() { return FieldOffsetX; }
    inline double GetFieldOffsetY() { return FieldOffsetY; }
    inline double GetFieldOffsetZ() { return FieldOffsetZ; }
    inline double GetGladAngle() { return GladAngle; }
    inline double GetGladOffsetX() { return GladOffsetX; }
    inline double GetGladOffsetY() { return GladOffsetY; }
    inline double GetGladOffsetZ() { return GladOffsetZ; }

    void Print();

  protected:
    // initial conditions
		double Gain, Theta;
		double NoiseRMS, TimeBinSize, ShapingTime;
		double Threshold;
    double ActiveRegionx, ActiveRegiony, ActiveRegionz;
    double TPCLx, TPCLy, TPCLz, FrameThickness;
    double Windowx, Windowy, Windowz;
    double LongDiff, TransDiff, FanoFactor, EIonization, DriftVelocity;
    int PadSize;
    double TargetRadius, TargetLength, TargetAngle, TargetOffsetX, TargetOffsetY, TargetOffsetZ;
    double GladAngle, GladOffsetX, GladOffsetY, GladOffsetZ;
    double FieldAngle, FieldOffsetX, FieldOffsetY, FieldOffsetZ;
    TString OutputFileName;

    ClassDef(R3BGTPCSetup, 1)
};

#endif
