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

#ifndef R3BGTPCCALDATA_H
#define R3BGTPCCALDATA_H

#include "TObject.h"
#include <stdint.h>

class R3BGTPCCalData : public TObject
{

  public:
    // Default Constructor
    R3BGTPCCalData();

    /** Standard Constructor
     *@param padId               Crystal unique identifier
     *@param adc                  Calibrated adc energies
     **/
    R3BGTPCCalData(UShort_t padId,
                   std::vector<UShort_t>  adc);

    // Destructor
    virtual ~R3BGTPCCalData() {}

    // Getters
    inline const UShort_t& GetPadId() const { return fPadId; }
    inline const std::vector<UShort_t>& GetEnergy() const { return fADC; }

  protected:
    UShort_t fPadId;          // Pad unique identifier
    std::vector<UShort_t> fADC;    // ADC measurements, variable time bucket

  public:
    ClassDef(R3BGTPCCalData, 1)
};

#endif
