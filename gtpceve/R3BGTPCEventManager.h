#ifndef R3BGTPCEVENTMANAGER_H
#define R3BGTPCEVENTMANAGER_H

#include "FairEventManager.h"
#include "TEveEventManager.h"
#include "TGCanvas.h"
#include "TGNumberEntry.h"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairTask.h"

#include "TCanvas.h"
#include "TFile.h"

class TGListTreeItem;

class R3BGTPCEventManager : public TEveEventManager
{

  public:
    static R3BGTPCEventManager* Instance();
    R3BGTPCEventManager();
    virtual ~R3BGTPCEventManager();

    virtual void GotoEvent(Int_t event); ///< *MENU*
    virtual void NextEvent();            ///< *MENU*
    virtual void PrevEvent();            ///< *MENU*
    virtual void make_gui();
    virtual void SelectEvent();
    // virtual void Select3DThres();

    void AddTask(FairTask* task);
    virtual void Init(Int_t option = 1, Int_t level = 3, Int_t nNodes = 10000);

    virtual Int_t GetCurrentEvent() { return fEntry; }

    TCanvas* GetCvsPadPlane() { return fCvsPadPlane; }

    void RunEvent();

  private:
    FairRootManager* fRootManager;
    FairRunAna* fRunAna;

    Int_t fEntry;
    TGListTreeItem* fEvent;
    TGNumberEntry* fCurrentEvent;

    static R3BGTPCEventManager* fInstance;

    // Canvas
    TCanvas* fCvsPadPlane;
    TCanvas* fPadWave;

    R3BGTPCEventManager(const R3BGTPCEventManager&);
    R3BGTPCEventManager& operator=(const R3BGTPCEventManager&);

    ClassDef(R3BGTPCEventManager, 1);
};

#endif
