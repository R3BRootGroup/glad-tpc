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

class R3BGTPCEventManager;

// GLAD-TPC classes
#include "R3BGTPCHitData.h"
#include "R3BGTPCMap.h"
#include "R3BGTPCTrackData.h"

// FairRoot classes
#include "FairTask.h"

// ROOT classes
class TCanvas;
class TClonesArray;
class TEvePointSet;
class TEveGeoShape;
class TEveBoxSet;
class TEveLine;
class TGraph;
class TH1I;
class TH1D;
class TH1F;
class TH2D;
class TH2F;
class TH2Poly;
class TH3F;
class TPaletteAxis;
class TVector3;

#include <Rtypes.h>
#include <fstream>

class R3BGTPCEventDrawTask : public FairTask
{

  public:
    R3BGTPCEventDrawTask();
    R3BGTPCEventDrawTask(TString modes);

    ~R3BGTPCEventDrawTask();

    InitStatus Init();
    void Exec(Option_t* option);
    void Reset();

  private:
    R3BGTPCEventManager* fEventManager;
    R3BGTPCMap* fMap;

    // Canvases and histograms
    TCanvas* fCvsPadPlane;
    TH2Poly* fPadPlane;
    TCanvas* fCvsPadWave;
    TH1I* fPadWave;

    // Data containers
    TClonesArray* fHitCA;
    TClonesArray* fTrackCA;

    // Points
    TEveBoxSet* fHitBoxSet;
    TEvePointSet* fHitSet;
    std::vector<TEvePointSet*> fTrackHitSet;
    std::vector<TEveBoxSet*> fHitClusterSet;

    // Drawing functions
    void DrawHitPoints();
    void DrawPadPlane();
    void DrawTracks();

    // Updating
    void UpdateCvsPadPlane();

    // Colors
    EColor GetTrackColor(int i);

    ClassDef(R3BGTPCEventDrawTask, 1);
};
