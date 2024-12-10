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
    LOG(info) << "R3BGTPCGeoPar::putParams() called";
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
    /* list->add("GTPCTPCCenterX", TPCCenterX);
     list->add("GTPCTPCCenterY", TPCCenterY);
     list->add("GTPCTPCCenterZ", TPCCenterZ);*/
    list->add("GTPCFrameThickness", FrameThickness);
    list->add("GTPCWindowx", Windowx);
    list->add("GTPCWindowy", Windowy);
    list->add("GTPCWindowz", Windowz);
    list->add("GTPCPadSize", PadSize);
    list->add("GTPCTargetRadius", TargetRadius);
    list->add("GTPCTargetLength", TargetLength);
    list->add("GTPCTargetAngle", TargetAngle);
    /*list->add("GTPCTargetX", TargetX);
    list->add("GTPCTargetY", TargetY);
    list->add("GTPCTargetZ", TargetZ);*/
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
    LOG(info) << "R3BCalifR3BGTPCGeoParaMappingPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegionx", &ActiveRegionx)))
    {
        LOG(info) << "---Could not initialize GTPCActiveRegionx";
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegiony", &ActiveRegiony)))
    {
        LOG(info) << "---Could not initialize GTPCActiveRegiony";
        return kFALSE;
    }
    if (!(list->fill("GTPCActiveRegionz", &ActiveRegionz)))
    {
        LOG(info) << "---Could not initialize GTPCActiveRegionz";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLx", &TPCLx)))
    {
        LOG(info) << "---Could not initialize GTPCTPCLx";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLy", &TPCLy)))
    {
        LOG(info) << "---Could not initialize GTPCTPCLy";
        return kFALSE;
    }
    if (!(list->fill("GTPCTPCLz", &TPCLz)))
    {
        LOG(info) << "---Could not initialize GTPCTPCLz";
        return kFALSE;
    }
    /* if (!(list->fill("GTPCTPCCenterX", &TPCCenterX)))
     {
         LOG(info) << "---Could not initialize GTPCTPCCenterX";
         return kFALSE;
     }
     if (!(list->fill("GTPCTPCCenterY", &TPCCenterY)))
     {
         LOG(info) << "---Could not initialize GTPCTPCCenterY";
         return kFALSE;
     }
     if (!(list->fill("GTPCTPCCenterZ", &TPCCenterZ)))
     {
         LOG(info) << "---Could not initialize GTPCTPCCenterZ";
         return kFALSE;
     }*/
    if (!(list->fill("GTPCTargetOffsetX", &TargetOffsetX)))
    {
        LOG(info) << "---Could not initialize GTPCTargetOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetOffsetY", &TargetOffsetY)))
    {
        LOG(info) << "---Could not initialize GTPCTargetOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetOffsetZ", &TargetOffsetZ)))
    {
        LOG(info) << "---Could not initialize GTPCTargetOffsetZ";
        return kFALSE;
    }
    if (!(list->fill("GTPCFrameThickness", &FrameThickness)))
    {
        LOG(info) << "---Could not initialize GTPCFrameThickness";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowx", &Windowx)))
    {
        LOG(info) << "---Could not initialize GTPCWindowx";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowy", &Windowy)))
    {
        LOG(info) << "---Could not initialize GTPCWindowy";
        return kFALSE;
    }
    if (!(list->fill("GTPCWindowz", &Windowz)))
    {
        LOG(info) << "---Could not initialize GTPCWindowz";
        return kFALSE;
    }
    if (!(list->fill("GTPCPadSize", &PadSize)))
    {
        LOG(info) << "---Could not initialize GTPCPadSize";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetRadius", &TargetRadius)))
    {
        LOG(info) << "---Could not initialize GTPCTargetRadius";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetLength", &TargetLength)))
    {
        LOG(info) << "---Could not initialize GTPCTargetLength";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetAngle", &TargetAngle)))
    {
        LOG(info) << "---Could not initialize GTPCTargetAngle";
        return kFALSE;
    }
    /*if (!(list->fill("GTPCTargetX", &TargetX)))
    {
        LOG(info) << "---Could not initialize GTPCTargetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetY", &TargetY)))
    {
        LOG(info) << "---Could not initialize GTPCTargetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCTargetZ", &TargetZ)))
    {
        LOG(info) << "---Could not initialize GTPCTargetZ";
        return kFALSE;
    }*/
    if (!(list->fill("GTPCGladAngle", &GladAngle)))
    {
        LOG(info) << "---Could not initialize GTPCGladAngle";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetX", &GladOffsetX)))
    {
        LOG(info) << "---Could not initialize GTPCGladOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetY", &GladOffsetY)))
    {
        LOG(info) << "---Could not initialize GTPCGladOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCGladOffsetZ", &GladOffsetZ)))
    {
        LOG(info) << "---Could not initialize GTPCGladOffsetZ";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldAngle", &FieldAngle)))
    {
        LOG(info) << "---Could not initialize GTPCFieldAngle";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetX", &FieldOffsetX)))
    {
        LOG(info) << "---Could not initialize GTPCFieldOffsetX";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetY", &FieldOffsetY)))
    {
        LOG(info) << "---Could not initialize GTPCFieldOffsetY";
        return kFALSE;
    }
    if (!(list->fill("GTPCFieldOffsetZ", &FieldOffsetZ)))
    {
        LOG(info) << "---Could not initialize GTPCFieldOffsetZ";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method print ----------------------------------------------------------
void R3BGTPCGeoPar::print() { printParams(); }

// ----  Method printParams ----------------------------------------------------
void R3BGTPCGeoPar::printParams()
{
    LOG(info) << "R3BGTPCGeoPar: GTPC Geo Parameters:";

    LOG(info) << "GTPCActiveRegionx " << ActiveRegionx << " cm,  "
              << "GTPCActiveRegiony " << ActiveRegiony << " cm,  "
              << "GTPCActiveRegionz " << ActiveRegionz << " cm" << endl;
    LOG(info) << "GTPCTPCLx " << TPCLx << " cm,  "
              << "GTPCTPCLy " << TPCLy << " cm,  "
              << "GTPCTPCLz " << TPCLz << " cm,  "
              << "GTPCTargetOffsetX " << TargetOffsetX << " cm,  "
              << "GTPCTargetOffsetY " << TargetOffsetY << " cm,  "
              << "GTPCTargetOffsetZ " << TargetOffsetZ
              << " cm,	 "
              /* << "GTPCTPCCenterX " << TPCCenterX << " cm,  "
               << "GTPCTPCCenterY " << TPCCenterY << " cm,  "
               << "GTPCTPCCenterZ " << TPCCenterZ << " cm,  "*/
              << "GTPCFrameThickness " << FrameThickness << " cm" << endl;
    LOG(info) << "GTPCWindowx " << Windowx << " cm,  "
              << "GTPCWindowy " << Windowy << " cm,  "
              << "GTPCWindowz " << Windowz << " cm,  "
              << "GTPCPadSize " << PadSize << " arbitrary units (virtual)" << endl;
    LOG(info) << "GTPCTargetRadius " << TargetRadius << " cm,  "
              << "GTPCTargetLength " << TargetLength << " cm,  "
              << "GTPCTargetAngle " << TargetAngle
              << " deg,  "
              /* << "GTPCTargetX " << TargetX << " cm,  "
               << "GTPCTargetY " << TargetY << " cm,  "
               << "GTPCTargetZ " << TargetZ << " cm"*/
              << endl;
    LOG(info) << "GTPCGladAngle " << GladAngle << " deg,  "
              << "GTPCGladOffsetX " << GladOffsetX << " cm,  "
              << "GTPCGladOffsetY " << GladOffsetY << " cm,  "
              << "GTPCGladOffsetZ " << GladOffsetZ << " cm" << endl;
    LOG(info) << "GTPCFieldAngle " << FieldAngle << " deg,  "
              << "GTPCFieldOffsetX " << FieldOffsetX << " cm,  "
              << "GTPCFieldOffsetY " << FieldOffsetY << " cm,  "
              << "GTPCFieldOffsetZ " << FieldOffsetZ << " cm" << endl;
}

ClassImp(R3BGTPCGeoPar);
