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

#include "R3BGTPCGeoPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ----------------------------------------------
R3BGTPCGeoPar::R3BGTPCGeoPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
{
}

// ----  Destructor -------------------------------------------------------
R3BGTPCGeoPar::~R3BGTPCGeoPar() { clear(); }

// ----  Method clear ----------------------------------------------------------
void R3BGTPCGeoPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BGTPCGeoPar::putParams(FairParamList* list)
{
    LOG(INFO) << "R3BGTPCGeoPar::putParams() called";
    if (!list)
    {
        return;
    }
    list->add("GTPCActiveRegionx", ActiveRegionx);
    list->add("GTPCActiveRegiony", ActiveRegiony);
    list->add("GTPCActiveRegionz", ActiveRegionz);
    list->add("GTPCTPCLx", TPCLx);
    list->add("GTPCTPCLy", TPCLy);
    list->add("GTPCTPCLz", TPCLz);
    list->add("GTPCFrameThickness", FrameThickness);
    list->add("GTPCWindowx", Windowx);
    list->add("GTPCWindowy", Windowy);
    list->add("GTPCWindowz", Windowz);
    list->add("GTPCPadSize", PadSize);
    list->add("GTPCTargetRadius", TargetRadius);
    list->add("GTPCTargetLength", TargetLength);
    list->add("GTPCTargetAngle", TargetAngle);
    list->add("GTPCTargetOffsetX", TargetOffsetX);
    list->add("GTPCTargetOffsetY", TargetOffsetY);
    list->add("GTPCTargetOffsetZ", TargetOffsetZ);
    list->add("GTPCGladAngle", GladAngle);
    list->add("GTPCGladOffsetX", GladOffsetX);
    list->add("GTPCGladOffsetY", GladOffsetY);
    list->add("GTPCGladOffsetZ", GladOffsetZ);
    list->add("GTPCFieldAngle", FieldAngle);
    list->add("GTPCFieldOffsetX", FieldOffsetX);
    list->add("GTPCFieldOffsetY", FieldOffsetY);
    list->add("GTPCFieldOffsetZ", FieldOffsetZ);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BGTPCGeoPar::getParams(FairParamList* list)
{
    LOG(INFO) << "R3BCalifR3BGTPCGeoParaMappingPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegionx", &ActiveRegionx)))
    {
        LOG(INFO) << "---Could not initialize GTPCActiveRegionx";
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegiony", &ActiveRegiony)))
    {
        LOG(INFO) << "---Could not initialize GTPCActiveRegiony";
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegionz", &ActiveRegionz)))
    {
        LOG(INFO) << "---Could not initialize GTPCActiveRegionz";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLx", &TPCLx)))
    {
        LOG(INFO) << "---Could not initialize GTPCTPCLx";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLy", &TPCLy)))
    {
        LOG(INFO) << "---Could not initialize GTPCTPCLy";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLz", &TPCLz)))
    {
        LOG(INFO) << "---Could not initialize GTPCTPCLz";
        return kFALSE;
    }
    if (!(list->fill("GTPCFrameThickness", &FrameThickness)))
    {
        LOG(INFO) << "---Could not initialize GTPCFrameThickness";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowx", &Windowx)))
    {
        LOG(INFO) << "---Could not initialize GTPCWindowx";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowy", &Windowy)))
    {
        LOG(INFO) << "---Could not initialize GTPCWindowy";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowz", &Windowz)))
    {
        LOG(INFO) << "---Could not initialize GTPCWindowz";
        return kFALSE;
    }
    if (!(list->fill("GTPCPadSize", &PadSize)))
    {
        LOG(INFO) << "---Could not initialize GTPCPadSize";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetRadius", &TargetRadius)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetRadius";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetLength", &TargetLength)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetLength";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetAngle", &TargetAngle)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetAngle";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetOffsetX", &TargetOffsetX)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetOffsetY", &TargetOffsetY)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetOffsetZ", &TargetOffsetZ)))
    {
        LOG(INFO) << "---Could not initialize GTPCTargetOffsetZ";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladAngle", &GladAngle)))
    {
        LOG(INFO) << "---Could not initialize GTPCGladAngle";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetX", &GladOffsetX)))
    {
        LOG(INFO) << "---Could not initialize GTPCGladOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetY", &GladOffsetY)))
    {
        LOG(INFO) << "---Could not initialize GTPCGladOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetZ", &GladOffsetZ)))
    {
        LOG(INFO) << "---Could not initialize GTPCGladOffsetZ";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldAngle", &FieldAngle)))
    {
        LOG(INFO) << "---Could not initialize GTPCFieldAngle";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetX", &FieldOffsetX)))
    {
        LOG(INFO) << "---Could not initialize GTPCFieldOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetY", &FieldOffsetY)))
    {
        LOG(INFO) << "---Could not initialize GTPCFieldOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetZ", &FieldOffsetZ)))
    {
        LOG(INFO) << "---Could not initialize GTPCFieldOffsetZ";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCGeoPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCGeoPar::printParams()
{
    LOG(INFO) << "R3BGTPCGeoPar: GTPC Geo Parameters:";

    LOG(INFO) << "GTPCActiveRegionx " << ActiveRegionx << " cm,  "
              << "GTPCActiveRegiony " << ActiveRegiony << " cm,  "
              << "GTPCActiveRegionz " << ActiveRegionz << " cm" << endl;
    LOG(INFO) << "GTPCTPCLx " << TPCLx << " cm,  "
              << "GTPCTPCLy " << TPCLy << " cm,  "
              << "GTPCTPCLz " << TPCLz << " cm,  "
              << "GTPCFrameThickness " << FrameThickness << " cm" << endl;
    LOG(INFO) << "GTPCWindowx " << Windowx << " cm,  "
              << "GTPCWindowy " << Windowy << " cm,  "
              << "GTPCWindowz " << Windowz << " cm,  "
              << "GTPCPadSize " << PadSize << " arbitrary units (virtual)" << endl;
    LOG(INFO) << "GTPCTargetRadius " << TargetRadius << " cm,  "
              << "GTPCTargetLength " << TargetLength << " cm,  "
              << "GTPCTargetAngle " << TargetAngle << " deg,  "
              << "GTPCTargetOffsetX " << TargetOffsetX << " cm,  "
              << "GTPCTargetOffsetY " << TargetOffsetY << " cm,  "
              << "GTPCTargetOffsetZ " << TargetOffsetZ << " cm" << endl;
    LOG(INFO) << "GTPCGladAngle " << GladAngle << " deg,  "
              << "GTPCGladOffsetX " << GladOffsetX << " cm,  "
              << "GTPCGladOffsetY " << GladOffsetY << " cm,  "
              << "GTPCGladOffsetZ " << GladOffsetZ << " cm" << endl;
    LOG(INFO) << "GTPCFieldAngle " << FieldAngle << " deg,  "
              << "GTPCFieldOffsetX " << FieldOffsetX << " cm,  "
              << "GTPCFieldOffsetY " << FieldOffsetY << " cm,  "
              << "GTPCFieldOffsetZ " << FieldOffsetZ << " cm" << endl;
}

ClassImp(R3BGTPCGeoPar);
