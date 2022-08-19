#ifndef R3BGTPCEVENTDRAWTASK_H
#define R3BGTPCEVENTDRAWTASK_H

class R3BGTPCEventManager;

// GLAD-TPC classes
#include "R3BGTPCHitData.h"
#include "R3BGTPCTrackData.h"
#include "R3BGTPCMap.h"

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
    std::vector<TEvePointSet *> fTrackHitSet;
    std::vector<TEveBoxSet *> fHitClusterSet;
  
    // Drawing functions
    void DrawHitPoints();
    void DrawPadPlane();
    void DrawTracks();
  
    // Updating
    void UpdateCvsPadPlane();

    //Colors
    EColor GetTrackColor(int i);
  
    ClassDef(R3BGTPCEventDrawTask, 1);
};

#endif
