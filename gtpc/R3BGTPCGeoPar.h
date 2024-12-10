/******************************************************************************
 *   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2018-2025 Members of R3B Collaboration                     *
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

#include "FairLogger.h"
#include "FairParGenericSet.h"
#include "TMath.h"
#include "TObject.h"

using namespace std;

class FairParamList;

class R3BGTPCGeoPar : public FairParGenericSet
{

  public:
    R3BGTPCGeoPar(const char* name = "GTPCGeoPar",
                  const char* title = "GTPC Geometric Parameters",
                  const char* context = "GTPCGeoParContext");

    ~R3BGTPCGeoPar();

    /** Reset all parameters **/
    virtual void clear();

    /** Store all parameters using FairRuntimeDB **/
    virtual void putParams(FairParamList* list);

    /** Retrieve all parameters using FairRuntimeDB**/
    Bool_t getParams(FairParamList* list);

    /** Print values of parameters to the standard output **/
    virtual void print();
    void printParams();

    /** Accessor functions **/
    // Target__________________________________________________________________________
    const Double_t GetTargetLength() { return TargetLength; }
    const Double_t GetTargetRadius() { return TargetRadius; }
    const Double_t GetTargetAngle() { return TargetAngle; }
    /* const Double_t GetTargetX() { return TargetX; }
     const Double_t GetTargetY() { return TargetY; }
     const Double_t GetTargetZ() { return TargetZ; }*/
    const Double_t GetTargetOffsetX() { return TargetOffsetX; }
    const Double_t GetTargetOffsetY() { return TargetOffsetY; }
    const Double_t GetTargetOffsetZ() { return TargetOffsetZ; }
    // TPC_____________________________________________________________________________
    const Double_t GetActiveRegionx() { return ActiveRegionx; }
    const Double_t GetActiveRegiony() { return ActiveRegiony; }
    const Double_t GetActiveRegionz() { return ActiveRegionz; }
    const Double_t GetTPCLx() { return TPCLx; }
    const Double_t GetTPCLy() { return TPCLy; }
    const Double_t GetTPCLz() { return TPCLz; }
    /* const Double_t GetTPCCenterX() { return TPCCenterX; }
     const Double_t GetTPCCenterY() { return TPCCenterY; }
     const Double_t GetTPCCenterZ() { return TPCCenterZ; }*/
    const Double_t GetFrameThickness() { return FrameThickness; }
    const Double_t GetWindowx() { return Windowx; }
    const Double_t GetWindowy() { return Windowy; }
    const Double_t GetWindowz() { return Windowz; }
    const Int_t GetPadSize() { return PadSize; }
    // GLAD____________________________________________________________________________
    const Double_t GetFieldAngle() { return FieldAngle; }
    const Double_t GetFieldOffsetX() { return FieldOffsetX; }
    const Double_t GetFieldOffsetY() { return FieldOffsetY; }
    const Double_t GetFieldOffsetZ() { return FieldOffsetZ; }
    const Double_t GetGladAngle() { return GladAngle; }
    const Double_t GetGladOffsetX() { return GladOffsetX; }
    const Double_t GetGladOffsetY() { return GladOffsetY; }
    const Double_t GetGladOffsetZ() { return GladOffsetZ; }
    const Int_t GetDetectorType() { return DetectorType; }

    // Target__________________________________________________________________________
    void SetTargetLength(Double_t value) { TargetLength = value; }
    void SetTargetRadius(Double_t value) { TargetRadius = value; }
    void SetTargetAngle(Double_t value) { TargetAngle = value; }
    /* void SetTargetX(Double_t value) { TargetX = value; }
     void SetTargetY(Double_t value) { TargetY = value; }
     void SetTargetZ(Double_t value) { TargetZ = value; }*/
    void SetTargetOffsetX(Double_t value) { TargetOffsetX = value; }
    void SetTargetOffsetY(Double_t value) { TargetOffsetY = value; }
    void SetTargetOffsetZ(Double_t value) { TargetOffsetZ = value; }
    // TPC_____________________________________________________________________________
    void SetActiveRegionx(Double_t value) { ActiveRegionx = value; }
    void SetActiveRegiony(Double_t value) { ActiveRegiony = value; }
    void SetActiveRegionz(Double_t value) { ActiveRegionz = value; }
    void SetTPCLx(Double_t value) { TPCLx = value; }
    void SetTPCLy(Double_t value) { TPCLy = value; }
    void SetTPCLz(Double_t value) { TPCLz = value; }
    /* void SetTPCCenterX(Double_t value) { TPCCenterX = value; }
     void SetTPCCenterY(Double_t value) { TPCCenterY = value; }
     void SetTPCCenterZ(Double_t value) { TPCCenterZ = value; }*/
    void SetFrameThickness(Double_t value) { FrameThickness = value; }
    void SetWindowx(Double_t value) { Windowx = value; }
    void SetWindowy(Double_t value) { Windowy = value; }
    void SetWindowz(Double_t value) { Windowz = value; }
    void SetPadSize(Int_t value) { PadSize = value; }

    // GLAD____________________________________________________________________________
    void SetFieldAngle(Double_t value) { FieldAngle = value; }
    void SetFieldOffsetX(Double_t value) { FieldOffsetX = value; }
    void SetFieldOffsetY(Double_t value) { FieldOffsetY = value; }
    void SetFieldOffsetZ(Double_t value) { FieldOffsetZ = value; }
    void SetGladAngle(Double_t value) { GladAngle = value; }
    void SetGladOffsetX(Double_t value) { GladOffsetX = value; }
    void SetGladOffsetY(Double_t value) { GladOffsetY = value; }
    void SetGladOffsetZ(Double_t value) { GladOffsetZ = value; }
    void SetDetectorType(Int_t value) { DetectorType = value; }

  private:
    Double_t ActiveRegionx;       // [cm]
    Double_t ActiveRegiony;       // [cm]
    Double_t ActiveRegionz;       // [cm]
    Double_t TPCLx, TPCLy, TPCLz; // [cm]
    // Double_t TPCCenterX, TPCCenterY, TPCCenterZ;          // [cm]
    Double_t FrameThickness;                              // [cm]
    Double_t Windowx, Windowy, Windowz;                   // [cm]
    Int_t PadSize;                                        // virtual: 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    Double_t TargetRadius;                                // [cm]
    Double_t TargetLength;                                // [cm]
    Double_t TargetAngle;                                 // [cm]
    Double_t TargetX, TargetY, TargetZ;                   // [cm]
    Double_t TargetOffsetX, TargetOffsetY, TargetOffsetZ; // [cm]
    Double_t GladAngle;                                   // [deg]
    Double_t GladOffsetX, GladOffsetY, GladOffsetZ;       // [cm]
    Double_t FieldAngle;                                  // [deg]
    Double_t FieldOffsetX, FieldOffsetY, FieldOffsetZ;    // [cm]
    Int_t DetectorType;                                   // 1 for prototype, 2 for FullBeamIn, 3 for FullBeamOut

    const R3BGTPCGeoPar& operator=(const R3BGTPCGeoPar&); /*< an assignment operator>*/
    R3BGTPCGeoPar(const R3BGTPCGeoPar&);                  /*< a copy constructor >*/

    ClassDef(R3BGTPCGeoPar, 1)
};
