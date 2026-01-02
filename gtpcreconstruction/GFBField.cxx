#include "GFBField.h"

// #include "R3BGladFieldMap.h"
#include <iostream>

ClassImp(GFBField)

    GFBField* GFBField::fInstance = nullptr;

GFBField::GFBField(const char* mapName, const char* fileType, Double_t xOffset, Double_t yOffset, Double_t zOffset)
{
    fFieldMap = new R3BGladFieldMap(mapName, fileType);
    fFieldMap->SetPosition(xOffset, yOffset, zOffset);
    fFieldMap->Init();

    std::cout << "== [GFBField] Field center : (" << xOffset << ", " << yOffset << ", " << zOffset << ") cm"
              << std::endl;
}

GFBField* GFBField::GetInstance(const char* mapName,
                                const char* fileType,
                                Double_t xOffset,
                                Double_t yOffset,
                                Double_t zOffset)
{
    if (fInstance == nullptr)
        fInstance = new GFBField(mapName, fileType, xOffset, yOffset, zOffset);
    else
        std::cout << "== [GFBField] Existing pointer is returned" << std::endl;

    return fInstance;
}

TVector3 GFBField::get(const TVector3& pos) const
{
    auto pos_m = TVector3(pos.X(), pos.Y(), pos.Z());

    auto bx = fFieldMap->GetBx(pos_m.X(), pos_m.Y(), pos_m.Z());
    auto by = fFieldMap->GetBy(pos_m.X(), pos_m.Y(), pos_m.Z());
    auto bz = fFieldMap->GetBz(pos_m.X(), pos_m.Y(), pos_m.Z());

    return TVector3(bx, by, bz);
}

Double_t GFBField::getX(const TVector3& pos) const
{
    auto pos_m = TVector3(pos.X(), pos.Y(), pos.Z());

    auto bx = fFieldMap->GetBx(pos_m.X(), pos_m.Y(), pos_m.Z());

    return bx;
}

Double_t GFBField::getY(const TVector3& pos) const
{
    auto pos_m = TVector3(pos.X(), pos.Y(), pos.Z());

    auto by = fFieldMap->GetBy(pos_m.X(), pos_m.Y(), pos_m.Z());

    return by;
}

Double_t GFBField::getZ(const TVector3& pos) const
{
    auto pos_m = TVector3(pos.X(), pos.Y(), pos.Z());

    auto bz = fFieldMap->GetBz(pos_m.X(), pos_m.Y(), pos_m.Z());

    return bz;
}
