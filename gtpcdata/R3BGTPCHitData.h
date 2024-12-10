/******************************************************************************
 *   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2019 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#ifndef R3BGTPCHITDATA_H
#define R3BGTPCHITDATA_H

#include "TObject.h"
#include <stdint.h>

class R3BGTPCHitData : public TObject
{

  public:
    // Default Constructor
    R3BGTPCHitData();

    /** Standard Constructor
     *@param x          X position of the hit in the gas
     *@param y          Y position of the hit in the gas
     *@param z          Z position of the hit in the gas
     *@param longWidth  Longitudinal width of electron cloud
     *@param energy     Total energy atributed to the hit
     **/
    R3BGTPCHitData(Double_t x, Double_t y, Double_t z, Double_t longWidth, Double_t energy);

    // Destructor
    virtual ~R3BGTPCHitData() {}

    // Getters
    inline const Double_t GetX() const { return fX; }
    inline const Double_t GetY() const { return fY; }
    inline const Double_t GetZ() const { return fZ; }
    inline const Double_t GetLongWidth() const { return fLongWidth; }
    inline const Double_t GetEnergy() const { return fEnergy; }
    inline const Int_t GetTime() const { return fTime; }

    // Setters
    inline void SetX(Double_t x) { fX = x; }
    inline void SetY(Double_t y) { fY = y; }
    inline void SetZ(Double_t z) { fZ = z; }
    inline void SetEnergy(Double_t E) { fEnergy = E; }
    inline void SetTime(Double_t T) { fTime = T; }

  protected:
    Double_t fX;         // X position of the hit in the gas
    Double_t fY;         // Y position of the hit in the gas
    Double_t fZ;         // Z position of the hit in the gas
    Double_t fLongWidth; // Longitudinal width of electron cloud
    Double_t fEnergy;    // Total energy atributed to the hit
    Int_t fTime;         // Time bucket

  public:
    ClassDef(R3BGTPCHitData, 1)
};

#endif
