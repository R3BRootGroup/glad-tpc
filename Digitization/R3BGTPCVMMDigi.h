/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 ******************************************************************************/

#pragma once

#include "TObject.h"

/**
 * R3BGTPCVMMDigi
 * ==============
 *
 * One VMM3a hit on one pad, as written by R3BGTPCVMMDigitizer.
 *
 * In continuous (self-triggered) mode the VMM3a delivers no waveform, only the
 * output of the peak detector (PDO, 10-bit ADC) and a time per hit
 * (de Geronimo et al., IEEE TNS 69 (2022) 976, Fig. 3). A digi therefore holds:
 *
 *   pad id          GTPC pad number (padId = column * nRows + row, see R3BGTPCMap)
 *   ADC             10-bit peak amplitude, 0 ... 1023, baseline subtracted.
 *                   The charge is ONLY given in ADC counts.
 *   time            the time the VMM3a measures [ns], absolute (event time included):
 *                   the threshold crossing or the peak, as the chip is configured
 *                   (R3BGTPCVMMDigitizer::SetTimeAtThreshold, stored in the run settings
 *                   as VMM3a_time_at_threshold). The chip delivers ONE time per hit, as in
 *                   the ATLAS MM digitization (vmmReadoutMode "peak" / "threshold").
 *                   Neighbour-logic digis: the time of the channel that triggered them.
 *   saturated       the amplitude was clipped at 1023
 *   neighbour       read out by the VMM3a neighbour logic, below threshold itself
 *   noise           made by the electronics noise alone (Rice formula), no detector charge
 */
class R3BGTPCVMMDigi : public TObject
{
  public:
    static constexpr Int_t kADCMax = 1023; // 10-bit ADC

    R3BGTPCVMMDigi() = default;
    R3BGTPCVMMDigi(Int_t padId, UShort_t adc, Double_t timeNs, Bool_t saturated, Bool_t neighbor, Bool_t noise)
        : fPadId(padId)
        , fADC(adc)
        , fTimeNs(timeNs)
        , fSaturated(saturated)
        , fNeighbor(neighbor)
        , fNoise(noise)
    {
    }
    ~R3BGTPCVMMDigi() override = default;

    Int_t GetPadId() const { return fPadId; }
    UShort_t GetADC() const { return fADC; }
    Double_t GetTime() const { return fTimeNs; } // measured time [ns] (threshold or peak, see above)
    Bool_t IsSaturated() const { return fSaturated; }
    Bool_t IsNeighborTriggered() const { return fNeighbor; }
    Bool_t IsNoise() const { return fNoise; }

    void Print(const Option_t* option = "") const override;

  private:
    Int_t fPadId = -1;
    UShort_t fADC = 0;
    Double_t fTimeNs = 0.;
    Bool_t fSaturated = kFALSE;
    Bool_t fNeighbor = kFALSE;
    Bool_t fNoise = kFALSE;

  public:
    ClassDefOverride(R3BGTPCVMMDigi, 1) //
};
