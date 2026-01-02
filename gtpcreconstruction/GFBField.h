#pragma once

#include <FairLogger.h>
#include <FairRootManager.h>
// #include <FairRunAna.h>
// #include <FairRuntimeDb.h>
#include "../field/R3BGladFieldMap.h"
#include "AbsBField.h"
#include "TObject.h"
#include "TVector3.h"

class GFBField : public TObject, public genfit::AbsBField
{
  public:
    static GFBField* GetInstance(const char* mapName,
                                 const char* fileType = "A",
                                 Double_t xOffset = -0.1794,
                                 Double_t yOffset = -20.5502,
                                 Double_t zOffset = 58.0526);

    TVector3 get(const TVector3& pos) const;
    Double_t getX(const TVector3& pos) const;
    Double_t getY(const TVector3& pos) const;
    Double_t getZ(const TVector3& pos) const;

    R3BGladFieldMap* GetFieldMap() { return fFieldMap; }

  private:
    GFBField(const char* mapName,
             const char* fileType = "A",
             Double_t xOffset = -0.1794,
             Double_t yOffset = -20.5502,
             Double_t zOffset = 58.0526);

    // clang-format off
    virtual ~GFBField() {};
    // clang-format on

    static GFBField* fInstance;
    R3BGladFieldMap* fFieldMap;

    ClassDef(GFBField, 1)
};
