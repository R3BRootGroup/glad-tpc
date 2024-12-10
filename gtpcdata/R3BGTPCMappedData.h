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

#ifndef R3BGTPCMAPPEDDATA_H
#define R3BGTPCMAPPEDDATA_H

#include "TObject.h"
#include <stdint.h>

class R3BGTPCMappedData : public TObject
{

  public:
    // Default Constructor
    R3BGTPCMappedData();

    /** Standard Constructor
     *@param padId               Crystal unique identifier
     *@param adc                  Vector of ADC measurements, variable size
     *@param isValid              Data validity check
     *@param isPedestalSubtracted Pedestal subtraction flag
     **/
    R3BGTPCMappedData(UShort_t padId, std::vector<UShort_t> adc, Bool_t isValid, Bool_t isPedestalSubtracted);

    // Destructor
    virtual ~R3BGTPCMappedData() {}

    // Getters
    inline const UShort_t& GetPadId() const { return fPadId; }
    inline const std::vector<UShort_t>& GetADC() const { return fADC; }
    inline const Bool_t& IsValid() const { return fIsValid; }
    inline const Bool_t& IsPedestalSubtracted() const { return fIsPedestalSubtracted; }

  protected:
    UShort_t fPadId;              // Pad unique identifier
    std::vector<UShort_t> fADC;   // ADC measurements, variable time bucket
    Bool_t fIsValid;              // Valid check NEEDED??
    Bool_t fIsPedestalSubtracted; // Needed? REMOVE ME IF IT IS A CTE.
                                  // CHARACTERISTIC OF DATA

  public:
    ClassDef(R3BGTPCMappedData, 1)
};

#endif
