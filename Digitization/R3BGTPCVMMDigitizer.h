/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 ******************************************************************************/

#pragma once

/**
 * R3BGTPCVMMDigitizer
 * ===================
 *
 * Event-by-event digitisation of the GLAD/HYDRA TPC pad plane read out by the
 * VMM3a in continuous (self-triggered) mode.
 *
 *   Input : GTPCProjPoint     arrival time [ns] and pad of every drift electron
 *                             (R3BTPCProjector, ProjPoint mode)
 *           GTPCMap           pad geometry used by the projector
 *           GTPCGasPar        drift velocity, and the TGeo Active_region (drift length):
 *                             together the readout window of one event (noise hits)
 *           event header      event time (FairEventHeader::GetEventTime, 0 by default):
 *                             digi times are absolute, as in CBM (digi time = event time + MC time)
 *   Output: GTPCVMMDigi       pad, 10-bit ADC, ONE time (threshold or peak, chip setting), flags
 *           GTPCVMMDigiMatch  MC truth of every digi (same index), weights in ADC counts:
 *                             MC links (input file id, MC entry, MC track) from the event
 *                             header; noise is a separate noise weight, not a link (R3BGTPCMatch)
 *
 * Units (the same in the projector, the pad map, this digitizer and the reconstruction):
 *   length   cm   (GTPCProjPoint landing points and R3BGTPCMap: plane coordinates of the readout face)
 *   time     ns   (arrival times, peaking / dead time, digi times)
 *   charge   ADC counts (after the one conversion below)
 *   gas      drift velocity cm/ns, diffusion coefficients cm^2/ns (GTPCGasPar)
 *   DLC      set in the units they are usually quoted in (SetDLC: ohm/square, um; SetDLCRC: ns/mm^2;
 *            SetDLCInitialSigma: mm) and written to the run settings in these units
 *            (VMM3a_DLC_RC_ns_per_mm2, VMM3a_DLC_sigma0_mm); converted once in Init() to
 *            RC [ns/cm^2] = 100 x RC [ns/mm^2] and sigma0 [cm] = sigma0 [mm] / 10, used everywhere inside.
 *
 * All amplitudes are in ADC counts. The input electronics settings are converted once,
 * in Init(), into one factor
 *     ADC per electron = e [fC] * G_VMM [mV/fC] * c_ADC [counts/mV]
 * and everything after that (pulses, noise, threshold, digis, MC truth) is in ADC.
 *
 * Chain per pad (references: [VMM] de Geronimo et al., IEEE TNS 69 (2022) 976;
 * [HYDRA] Enciu et al., arXiv:2606.22607; [Rice] S.O. Rice, BSTJ 23 (1944) 282,
 * as used in arXiv:1305.4592 and in the CBM-STS digitizer)
 *
 */

#include "FairTask.h"

#include <complex>
#include <deque>
#include <random>
#include <unordered_map>
#include <vector>

class TClonesArray;
class R3BGTPCMatch;
class R3BGTPCGasPar;

class R3BGTPCVMMDigitizer : public FairTask
{
  public:
    R3BGTPCVMMDigitizer();
    ~R3BGTPCVMMDigitizer() override;

    InitStatus Init() override;
    void SetParContainers() override;
    void Exec(Option_t*) override;
    void Finish() override;

    // ---- gas amplification: mean number of avalanche electrons per drift electron
    void SetDetectorGain(Double_t meanGain) { fDetectorGain = meanGain; }
    /** Polya gain fluctuations; theta = 0: exponential */
    void SetPolyaTheta(Double_t theta)
    {
        fUsePolya = kTRUE;
        fPolyaTheta = theta;
    }

    // ---- VMM3a settings (all required)
    void SetVMMGain(Double_t mVPerFC) { fVMMGainMVPerFC = mVPerFC; } // 0.5, 1, 3, 4.5, 6, 9, 12, 16 mV/fC
    void SetPeakingTime(Double_t ns) { fPeakingTimeNs = ns; }        // 25, 50, 100, 200 ns
    /** VMM3a dead time of a channel (VMM3 specifications v1.0, App. A, Figs. 21 and 23, 10-bit ADC data):
     *  the effective dead time starts at the RISING EDGE (threshold crossing) and ends deadTime after the
     *  PEAK, deadTime = 220 ns digitization (the 10-bit ADC converts the peak value) + 30 ns reset:
     *      rising edge |-- rise to the peak --| peak |-- 220 ns digitization --|-- 30 ns reset --| ready
     *  If the signal is still above threshold at the end of the digitization (time over threshold
     *  longer), the last reset dominates: the reset (resetTime) follows the falling edge:
     *      channel ready again at max(t_peak + deadTime, t_falling edge + resetTime)
     *  A new hit needs a new rising edge after that. The charge is the peak value. */
    void SetDeadTime(Double_t ns) { fDeadTimeNs = ns; }   // after the peak: 220 + 30 = 250 ns (VMM3a)
    void SetResetTime(Double_t ns) { fResetTimeNs = ns; } // part of it, also after a long falling edge: 30 ns
    void SetADCCountsPerMV(Double_t counts) { fADCCountsPerMV = counts; } // ADC slope (calibration)
    void SetENC(Double_t electronsRMS) { fENCElectrons = electronsRMS; }  // noise [e rms], HYDRA ~5000 e

    // ---- threshold above the baseline: give ONE of the two
    void SetThresholdSigma(Double_t nSigma)
    {
        fThresholdSigmaInput = nSigma;
        fThresholdMVInput = -1.;
    }
    void SetThresholdMV(Double_t mV)
    {
        fThresholdMVInput = mV;
        fThresholdSigmaInput = -1.;
    }

    // ---- electronics noise (on the signal and Rice noise hits)
    void EnableNoise(Bool_t on = kTRUE) { fNoiseEnabled = on; }
    /** VMM3a timing mode (chip register): time at the threshold crossing (kTRUE, default)
     *  or at the peak (kFALSE). Must match the configuration of the real chips. */
    void SetTimeAtThreshold(Bool_t on) { fTimeAtThreshold = on; }
    /** K_filter (dimensionless), R0 = K / t_p. Not set or <= 0: from the VMM3a shaper (0.519) */
    void SetNoiseKFilter(Double_t k) { fNoiseKFilter = k; }

    /** VMM3a neighbour logic: row +- 1 (next pad along X) and / or column +- 1 (along Z) */
    void SetNeighborLogic(Bool_t rows, Bool_t columns)
    {
        fNeighborRows = rows;
        fNeighborColumns = columns;
    }

    /** Resistive (DLC) anode, see 1b. R = sheet resistance of the DLC [ohm/square], d = thickness of
     *  the insulator between the DLC and the pads [um], epsR = its relative permittivity.
     *  RC = R eps0 epsR / d. (HYDRA: R = 1.2 - 1.5 MOhm/square measured, arXiv:2504.10213) */
    void SetDLC(Double_t ohmPerSquare, Double_t insulatorUm, Double_t epsR)
    {
        fDLCEnabled = kTRUE;
        fDLCOhmPerSquare = ohmPerSquare;
        fDLCInsulatorUm = insulatorUm;
        fDLCEpsR = epsR;
        fDLCRCInput = -1.;
    }
    /** Resistive (DLC) anode with RC [ns/mm^2] given directly (e.g. fitted to the data) */
    void SetDLCRC(Double_t nsPerMm2)
    {
        fDLCEnabled = kTRUE;
        fDLCRCInput = nsPerMm2;
    }
    /** width of the charge cloud of one avalanche when it reaches the DLC [mm] (default 0) */
    void SetDLCInitialSigma(Double_t mm) { fDLCSigma0Mm = mm; }

    void SetRandomSeed(ULong_t seed) { fRandomSeed = seed; }
    void SetPersistence(Bool_t on) { fPersistence = on; }

    // ---- derived values (valid after Init; the pad count after the first event)
    Double_t GetNoiseSigmaADC() const { return fNoiseSigmaADC; }
    Double_t GetThresholdADC() const { return fThresholdADC; }
    Double_t GetThresholdSigma() const { return fThresholdSigma; }
    Double_t GetADCPerDriftElectron() const { return fADCPerElectron * fDetectorGain; }
    Double_t GetDLCRCnsPerMm2() const { return fDLCEnabled ? fDLCRCnsPerMm2 : 0.; }
    Int_t GetNumberOfPads() const { return fNRows * fNColumns; }

  private:
    // shaper input of one grid step (t_{k-1}, t_k] of one pad, moved to t_k
    struct GridStep
    {
        Double_t realState = 0.;                  // charge for the real pole [ADC]
        std::complex<Double_t> complexState = 0.; // charge for the complex pole [ADC]
        Double_t currentChange = 0.;              // change of the input current at t_k [ADC/ns] (DLC)
        Bool_t hasInput = kFALSE;
    };

    // all shaper input of one pad in this event: grid steps firstStep, firstStep+1, ...
    struct PadInput
    {
        Long64_t firstStep = 0;                // grid step of steps[0]
        Long64_t usedFirst = 0, usedLast = -1; // first / last step with input
        std::vector<GridStep> steps;
        GridStep& At(Long64_t step); // creates the step if needed
        Bool_t Empty() const { return usedLast < usedFirst; }
    };

    // fast standard normal numbers (ziggurat) for the noise process
    struct Ziggurat
    {
        Ziggurat();
        Double_t operator()(std::mt19937_64& rng) const;
        UInt_t kn[128];
        Double_t wn[128], fn[128];
    };

    // signal + noise of one pad on the time grid: values at grid steps firstStep, firstStep + 1, ...
    struct Waveform
    {
        Long64_t firstStep = 0;
        std::vector<Double_t> values; // [ADC]
    };

    // one hit of a channel, before the common dead time with the noise hits
    struct Hit
    {
        Double_t crossingNs = -1.;  // threshold crossing [ns]
        Double_t peakNs = 0.;       // peak [ns]
        Double_t amplitudeADC = 0.; // peak detector output [ADC], noise included
        Bool_t noise = kFALSE;      // noise-only hit
        Double_t readyNs = 0.;      // channel ready again: max(peak + dead time, falling edge + reset) [ns]
    };

    // charge of one drift electron on a pad, for the MC truth
    struct PadElectron
    {
        Double_t timeNs;
        Double_t chargeADC;
        Int_t mcTrack;
    };

    // DLC: drift electrons with the same pad cell, landing-point bins, arrival grid step / phase and
    // MC track form one deposit (identical spreading)
    struct DLCDepositKey
    {
        Int_t row, col;     // pad cell of the landing point
        Int_t bx, bz;       // landing point within the cell (kDLCOffsetBins bins per axis)
        Int_t phase;        // arrival time within its grid step (kDLCPhaseBins bins)
        Long64_t firstStep; // first grid step after the arrival
        Int_t mcTrack;
        bool operator==(const DLCDepositKey& o) const
        {
            return row == o.row && col == o.col && bx == o.bx && bz == o.bz && phase == o.phase &&
                   firstStep == o.firstStep && mcTrack == o.mcTrack;
        }
    };
    struct DLCDepositKeyHash
    {
        std::size_t operator()(const DLCDepositKey& k) const;
    };
    struct DLCDeposit
    {
        Double_t chargeADC = 0.;               // sum of the avalanche charges [ADC]
        Double_t chargeTimeADCns = 0.;         // sum of charge x arrival time (absolute) [ADC ns]
        Double_t chargeUx = 0., chargeUz = 0.; // sum of charge x landing point within the cell (0 ... 1)
    };

    Bool_t CheckSettings() const;
    void PrepareShaper();
    Bool_t ReadPadGeometry();

    // ---- charge input
    Double_t AvalancheADC();
    void AddCharge(Int_t pad, Double_t timeNs, Double_t chargeADC);
    void AddChargeTo(PadInput& input, Double_t timeNs, Double_t chargeADC);

    // ---- resistive layer (DLC), lengths in cm, times in ns
    Double_t DLCSigmaCm(Double_t tauNs) const; // width of the charge on the DLC tau after the arrival
    void PrepareDLC();                         // time nodes, reach, tables of the pad shares
    void CollectDLCElectron(Double_t xCm, Double_t zCm, Double_t timeNs, Double_t chargeADC, Int_t mcTrack);
    void SpreadDLCDeposits(); // all deposits of the event -> shaper inputs of the pads
    void MergeTrackInputs();  // pad input = sum over its MC tracks
    void SignalOnGrid(const PadInput& input, Long64_t kFirst, Long64_t kLast, std::vector<Double_t>& out) const;

    // ---- channel response
    void PrepareNoise();                                                  // noise process on the time grid
    void NoiseSeries(std::size_t n, std::vector<Double_t>& out);          // n noise values [ADC]
    void DigitizePad(Int_t pad, PadInput& input, std::vector<Hit>& hits); // waveform -> hits
    void ChannelHits(const Waveform& wf, std::vector<Hit>& hits) const;   // VMM3a logic on a waveform
    Int_t GenerateNoise(Double_t tStartNs, Double_t tEndNs);              // Rice noise hits -> fNoiseHits
    void WriteHits(Int_t pad, std::vector<Hit>& hits);                    // common dead time -> digis
    Double_t ReadyTime(Double_t peakNs, Double_t fallingEdgeNs) const;    // end of the dead time of a hit
    void CreateDigi(Int_t pad,
                    Double_t amplitudeADC,
                    Double_t peakNs,
                    Double_t crossingNs,
                    Bool_t neighbor,
                    Bool_t noise);
    void AddNeighborDigis();

    // ---- MC truth
    Double_t ResponseAt(Double_t dtNs) const;
    void FillMatch(Int_t pad, Double_t peakTimeNs, R3BGTPCMatch& match) const;

    // ---- shaper helpers
    Double_t ShaperOutput(Double_t realState, const std::complex<Double_t>& complexState) const;
    Double_t ShaperSlope(Double_t realState, const std::complex<Double_t>& complexState) const;
    Double_t ShaperNoiseR0PerNs() const; // Rice R0 of white series noise through this shaper [1/ns]
    void MoveForward(Double_t& realState, std::complex<Double_t>& complexState, Double_t dt) const;

    // ---- I/O
    TClonesArray* fProjPoints = nullptr;  //!
    TClonesArray* fMapInput = nullptr;    //!
    TClonesArray* fDigis = nullptr;       //!
    TClonesArray* fDigiMatches = nullptr; //!
    Bool_t fPersistence = kTRUE;

    // ---- settings (negative = not set)
    Double_t fDetectorGain = -1.;
    Bool_t fUsePolya = kFALSE;
    Double_t fPolyaTheta = -1.;
    Double_t fVMMGainMVPerFC = -1.;
    Double_t fPeakingTimeNs = -1.;
    Double_t fDeadTimeNs = -1.;  // after the peak: digitization + reset [ns] (VMM3a: 220 + 30 = 250)
    Double_t fResetTimeNs = 30.; // channel reset [ns] (VMM3a: 30), after the falling edge if ToT is longer
    Double_t fADCCountsPerMV = -1.;
    Double_t fENCElectrons = -1.;
    Double_t fThresholdSigmaInput = -1.;
    Double_t fThresholdMVInput = -1.;
    Bool_t fNoiseEnabled = kFALSE;
    Bool_t fTimeAtThreshold = kTRUE;
    Double_t fNoiseKFilter = -1.;
    Bool_t fNeighborRows = kFALSE;
    Bool_t fNeighborColumns = kFALSE;
    ULong_t fRandomSeed = 12345;
    Bool_t fDLCEnabled = kFALSE;
    Double_t fDLCOhmPerSquare = -1.;
    Double_t fDLCInsulatorUm = -1.;
    Double_t fDLCEpsR = -1.;
    Double_t fDLCRCInput = -1.; // [ns/mm^2]
    Double_t fDLCSigma0Mm = 0.;

    // ---- derived in Init() [ADC unless stated]
    Double_t fADCPerElectron = 0.;              //! e * G_VMM * c_ADC
    Double_t fNoiseSigmaADC = 0.;               //! ENC in ADC
    Double_t fThresholdADC = 0.;                //! u
    Double_t fThresholdSigma = 0.;              //! u / sigma
    Double_t fStepNs = 0.;                      //! grid step = t_p / 100
    Double_t fRealRate = 0.;                    //! real pole [1/ns]
    std::complex<Double_t> fComplexRate;        //! complex pole [1/ns]
    Double_t fK1 = 0.;                          //! weights of the two terms, peak 1
    std::complex<Double_t> fK2;                 //!
    Double_t fNoiseKUsed = 0.;                  //! K_filter used
    Double_t fNoiseR0PerNs = 0.;                //! R0 [1/ns]
    Double_t fPeakJitterNs = 0.;                //! peak-time jitter = fPeakJitterNs x sigma_noise / A [ns]
    Double_t fNoiseRatePerNs = 0.;              //! R0 exp(-u^2/(2 sigma^2)) per channel [1/ns]
    Double_t fReadoutWindowNs = 0.;             //! drift length / drift velocity (geometry + GTPCGasPar)
    Double_t fNoisePhi[3][3] = {};              //! noise process: state transition over one grid step
    Double_t fNoiseLQ[3][3] = {};               //! Cholesky factor of the step covariance
    Double_t fNoiseLP[3][3] = {};               //! Cholesky factor of the stationary covariance
    Double_t fNoiseC[3] = {};                   //! output weights (rms = ENC)
    Double_t fEventTimeNs = 0.;                 //! time of the current event (event header)
    Double_t fCurrentRealGain = 0.;             //! real state after one grid step of unit input current
    std::complex<Double_t> fCurrentComplexGain; //! complex state after one grid step of unit current
    Double_t fDLCRCnsPerMm2 = 0.;               //! R C [ns/mm^2] (as set / written to the run settings)
    Double_t fDLCRCnsPerCm2 = 0.;               //! R C [ns/cm^2] (used inside)
    Double_t fDLCSigma0Cm = 0.;                 //! sigma0 [cm] (used inside)
    Int_t fDLCNodes = 0;                        //! time nodes: tau = 0 and phi + m step for m in fDLCNodeSteps
    std::vector<Long64_t> fDLCNodeSteps;        //!
    std::vector<Float_t> fDLCTableX,
        fDLCTableZ; //! pad shares [phase][node][landing point 0 ... kDLCOffsetBins][pad distance]
    std::vector<Float_t> fDLCMaxX, fDLCMaxZ; //! their maximum over the nodes [phase][landing bin][pad distance]
    Double_t fDLCMaxTimeNs = 0.;             //! the spreading is followed up to this tau
    R3BGTPCGasPar* fGasPar = nullptr;        //!

    // ---- pad geometry (from GTPCMap)
    Bool_t fGeometryRead = kFALSE; //!
    Int_t fNRows = 0;              //!
    Int_t fNColumns = 0;           //!
    Double_t fPitchXcm = 0.;       //!
    Double_t fPitchZcm = 0.;       //!
    Double_t fGapXcm = 0.;         //!
    Double_t fGapZcm = 0.;         //!
    Double_t fOffsetXcm = 0.;      //! first pad edge on the readout face
    Double_t fOffsetZcm = 0.;      //!
    Int_t fDLCReachRows = 0;       //! rows / columns the charge of one electron can reach
    Int_t fDLCReachColumns = 0;    //!

    // ---- per event
    std::unordered_map<Int_t, PadInput> fPads;                         //!
    std::unordered_map<Int_t, Waveform> fWaveforms;                    //! signal + noise of the pads with charge
    std::unordered_map<Int_t, std::vector<Hit>> fNoiseHits;            //!
    std::unordered_map<Int_t, std::vector<PadElectron>> fPadElectrons; //!
    std::unordered_map<Int_t, std::unordered_map<Int_t, PadInput>> fTrackInputs;   //! DLC: pad -> MC track -> input
    std::unordered_map<DLCDepositKey, DLCDeposit, DLCDepositKeyHash> fDLCDeposits; //! DLC deposits of the event
    std::vector<std::pair<Int_t, Hit>> fSelfHits; //! accepted self-triggered hits (neighbour logic: digis made there)
    std::mt19937_64 fRng;                         //! gas gain
    std::mt19937_64 fNoiseRng;                    //! electronics noise
    Ziggurat fGauss;                              //! normal numbers of the noise process

    // ---- MC event of the current entry (FairEventHeader), for the MC links
    Int_t fCurrentFile = 0;  //!
    Int_t fCurrentEntry = 0; //!

    // ---- counters
    Long64_t fNEvents = 0;
    Long64_t fNElectrons = 0;
    Long64_t fNElectronsNoTrack = 0;
    Long64_t fNDigis = 0;
    Long64_t fNSaturated = 0;
    Long64_t fNNeighborDigis = 0;
    Long64_t fNNoiseDigis = 0;
    Long64_t fNNoiseGenerated = 0;

  public:
    ClassDefOverride(R3BGTPCVMMDigitizer, 1)
};
