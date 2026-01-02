/******************************************************************************
 * Copyright (C) 2019-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH *
 *             Copyright (C) 2019-2026 Members of R3B Collaboration           *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU Lesser General Public Licence (LGPL) version 3,        *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#pragma once

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
    // Electronic______________________________________________________________________
    inline double GetGain() { return Gain; }
    inline double GetTheta() { return Theta; }
    inline double GetNoiseRMS() { return NoiseRMS; }
    inline double GetTimeBinSize() { return TimeBinSize; }
    inline double GetShapingTime() { return ShapingTime; }
    inline double GetThreshold() { return Threshold; }
    // Target__________________________________________________________________________
    inline double GetTargetLength() { return TargetLength; }
    inline double GetTargetRadius() { return TargetRadius; }
    inline double GetTargetAngle() { return TargetAngle; }
    inline double GetTargetX() { return TargetX; }
    inline double GetTargetY() { return TargetY; }
    inline double GetTargetZ() { return TargetZ; }
    // Plastic_wall____________________________________________________________________
    inline double GetScintX() { return ScintX; }
    inline double GetScintY() { return ScintY; }
    inline double GetScintZ() { return ScintZ; }
    // Fibers__________________________________________________________________________
    inline double GetFiberX() { return FiberX; }
    inline double GetFiberY() { return FiberY; }
    inline double GetFiberZ() { return FiberZ; }
    // Fibers IN GLAD__________________________________________________________________
    inline double GetFibINX() { return FibINX; }
    inline double GetFibINY() { return FibINY; }
    inline double GetFibINZ() { return FibINZ; }
    // TOFD____________________________________________________________________________
    inline double GetTOFDX() { return TOFDX; }
    inline double GetTOFDY() { return TOFDY; }
    inline double GetTOFDZ() { return TOFDZ; }
    // TPC_____________________________________________________________________________
    inline double GetActiveRegionx() { return ActiveRegionx; }
    inline double GetActiveRegiony() { return ActiveRegiony; }
    inline double GetActiveRegionz() { return ActiveRegionz; }
    inline double GetTPCLx() { return TPCLx; }
    inline double GetTPCLy() { return TPCLy; }
    inline double GetTPCLz() { return TPCLz; }
    inline double GetTPCCenterX() { return TPCCenterX; }
    inline double GetTPCCenterY() { return TPCCenterY; }
    inline double GetTPCCenterZ() { return TPCCenterZ; }
    inline double GetFrameThickness() { return FrameThickness; }
    inline double GetWindowx() { return Windowx; }
    inline double GetWindowy() { return Windowy; }
    inline double GetWindowz() { return Windowz; }
    // Drift___________________________________________________________________________
    inline double GetLongDiff() { return LongDiff; }
    inline double GetTransDiff() { return TransDiff; }
    inline double GetPadSize() { return PadSize; }
    inline double GetFanoFactor() { return FanoFactor; }
    inline double GetEIonization() { return EIonization; }
    inline double GetDriftVelocity() { return DriftVelocity; }
    // GLAD____________________________________________________________________________
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
    double TPCCenterX, TPCCenterY, TPCCenterZ;
    double Windowx, Windowy, Windowz;
    double LongDiff, TransDiff, FanoFactor, EIonization, DriftVelocity;
    int PadSize;
    double TargetRadius, TargetLength, TargetAngle, TargetX, TargetY, TargetZ;
    double ScintX, ScintY, ScintZ;
    double FiberX, FiberY, FiberZ;
    double FibINX, FibINY, FibINZ;
    double TOFDX, TOFDY, TOFDZ;
    double GladAngle, GladOffsetX, GladOffsetY, GladOffsetZ;
    double FieldAngle, FieldOffsetX, FieldOffsetY, FieldOffsetZ;
    TString OutputFileName;

    ClassDef(R3BGTPCSetup, 1)
};
