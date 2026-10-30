/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 ******************************************************************************/

#include "R3BGTPCVMMDigitizer.h"

#include "R3BGTPCGasPar.h"
#include "R3BGTPCMap.h"
#include "R3BGTPCMatch.h"
#include "R3BGTPCProjPoint.h"
#include "R3BGTPCVMMDigi.h"

#include "FairEventHeader.h"
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairSink.h"

#include "TClonesArray.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TMath.h"
#include "TParameter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <tuple>

namespace
{
    constexpr Double_t kElectronChargeFC = 1.602176634e-4; // only used once, for ADC per electron
    constexpr Double_t kTwoPi = 6.283185307179586;

    // VMM3a shaper poles in units of 1/alpha [VMM, appendix eq. 9]: one real pole, a complex pair
    constexpr Double_t kPoleReal = 1.263;
    constexpr Double_t kPoleRe = 1.149;
    constexpr Double_t kPoleIm = 0.789;

    // VMM3a settings that exist on the chip [VMM]
    constexpr std::array<Double_t, 8> kVMMGains = { 0.5, 1., 3., 4.5, 6., 9., 12., 16. }; // mV/fC
    constexpr std::array<Double_t, 4> kVMMPeakingTimes = { 25., 50., 100., 200. };        // ns
    constexpr Double_t kADCMax = R3BGTPCVMMDigi::kADCMax;

    // numerical controls only (not detector parameters)
    constexpr Double_t kStepsPerPeakingTime = 100.; // time grid = t_p / 100 (2 ns at 200 ns)
    constexpr Double_t kWindowBefore = 1.5;         // waveform of a pad: 1.5 t_p before its first charge ...
    constexpr Double_t kWindowAfter = 3.;           // ... to 3 t_p after its last charge

    // DLC (numerical controls only, see PrepareDLC):
    //  - the spreading of one avalanche is followed until its width is 1.5 pad pitches or until
    //    20 peaking times; later the charge left on a pad changes so slowly that it no longer changes
    //    the pulses (checked: following it to 3 pitches changes the pulses of the landing pad and of
    //    the pads up to two away by < 0.01 ADC of 480)
    //  - time nodes: every grid step at first, then +15 % of tau; between two nodes the pad charge
    //    changes linearly (constant current, integrated exactly by the shaper)
    //  - the pad shares are tabulated once: on kDLCOffsetBins + 1 landing points per pad cell and axis
    //    (every 31 um for 2 mm pads, linear interpolation in between) and kDLCPhaseBins arrival times
    //    within a grid step; drift electrons with the same pad cell, landing bin, grid step, arrival
    //    phase bin and MC track are added up before the spreading (charge-weighted landing point and
    //    time), e.g. the electrons of one ionisation cluster
    constexpr Double_t kEps0FPerM = 8.8541878128e-12;
    constexpr Double_t kDLCMaxSigmaInPitch = 1.5;
    constexpr Double_t kDLCMaxPeakingTimes = 20.;
    constexpr Double_t kDLCWindowNSigma = 4.;   // pads within +- 4 sigma(tau_max) of the landing point
    constexpr Double_t kDLCStepGrowth = 0.15;   // next node at tau + 15 % tau (at least one grid step)
    constexpr Double_t kDLCMinFraction = 1.e-3; // a pad that never holds more than this share of the
                                                // avalanche is skipped (its pulse would be < 1e-3 Q)
    constexpr Int_t kDLCOffsetBins = 64;
    constexpr Int_t kDLCPhaseBins = 4;

    // share of a 1D Gaussian (mean x, width sigma) inside [lo, hi]
    Double_t FractionInside(Double_t x, Double_t sigma, Double_t lo, Double_t hi)
    {
        if (!(sigma > 0.))
            return (x >= lo && x < hi) ? 1. : 0.;
        const Double_t s = 1. / (std::sqrt(2.) * sigma);
        return 0.5 * (std::erf((hi - x) * s) - std::erf((lo - x) * s));
    }
} // namespace

// ============================================================================
R3BGTPCVMMDigitizer::R3BGTPCVMMDigitizer()
    : FairTask("R3BGTPCVMMDigitizer", 1)
{
}

R3BGTPCVMMDigitizer::~R3BGTPCVMMDigitizer()
{
    if (fDigis)
    {
        fDigis->Delete();
        delete fDigis;
    }
    if (fDigiMatches)
    {
        fDigiMatches->Delete();
        delete fDigiMatches;
    }
}

// Shaper input of one pad: a vector over the grid steps with some room on both sides (inputs come in
// any time order), and the range of the steps that really have input (usedFirst ... usedLast)
R3BGTPCVMMDigitizer::GridStep& R3BGTPCVMMDigitizer::PadInput::At(Long64_t step)
{
    constexpr Long64_t kMargin = 64;
    if (steps.empty())
    {
        firstStep = step - kMargin;
        steps.resize(static_cast<std::size_t>(2 * kMargin + 1));
        usedFirst = usedLast = step;
    }
    else
    {
        if (step < firstStep)
        {
            const Long64_t n = firstStep - step + kMargin;
            steps.insert(steps.begin(), static_cast<std::size_t>(n), GridStep());
            firstStep -= n;
        }
        else if (step >= firstStep + static_cast<Long64_t>(steps.size()))
            steps.resize(static_cast<std::size_t>(step - firstStep + 1 + kMargin));
        usedFirst = std::min(usedFirst, step);
        usedLast = std::max(usedLast, step);
    }
    GridStep& gridStep = steps[static_cast<std::size_t>(step - firstStep)];
    gridStep.hasInput = kTRUE;
    return gridStep;
}

// ============================================================================
// Standard normal numbers, ziggurat method (G. Marsaglia, W.W. Tsang, J. Stat. Softw. 5 (2000) 8),
// 128 layers; the 32-bit value and the layer index come from separate bits of one 64-bit number.
// Several times faster than std::normal_distribution; used for the noise process (many values).
// ============================================================================
R3BGTPCVMMDigitizer::Ziggurat::Ziggurat()
{
    const Double_t m1 = 2147483648.;
    Double_t dn = 3.442619855899, tn = dn;
    const Double_t vn = 9.91256303526217e-3;
    const Double_t q = vn / std::exp(-0.5 * dn * dn);
    kn[0] = static_cast<UInt_t>((dn / q) * m1);
    kn[1] = 0;
    wn[0] = q / m1;
    wn[127] = dn / m1;
    fn[0] = 1.;
    fn[127] = std::exp(-0.5 * dn * dn);
    for (Int_t i = 126; i >= 1; --i)
    {
        dn = std::sqrt(-2. * std::log(vn / dn + std::exp(-0.5 * dn * dn)));
        kn[i + 1] = static_cast<UInt_t>((dn / tn) * m1);
        tn = dn;
        fn[i] = std::exp(-0.5 * dn * dn);
        wn[i] = dn / m1;
    }
}

Double_t R3BGTPCVMMDigitizer::Ziggurat::operator()(std::mt19937_64& rng) const
{
    auto uniform = [&rng]() { return (static_cast<Double_t>(rng() >> 11) + 0.5) * (1. / 9007199254740992.); }; // (0, 1)
    for (;;)
    {
        const ULong64_t r = rng();
        const Int_t hz = static_cast<Int_t>(static_cast<UInt_t>(r >> 32));
        const Int_t iz = static_cast<Int_t>(r & 127);
        const ULong64_t ahz = hz < 0 ? static_cast<ULong64_t>(-static_cast<Long64_t>(hz)) : static_cast<ULong64_t>(hz);
        if (ahz < kn[iz])
            return hz * wn[iz];
        // outside the rectangle of the layer
        const Double_t x = hz * wn[iz];
        if (iz == 0) // base layer: tail beyond r = 3.442620
        {
            constexpr Double_t r0 = 3.442620;
            Double_t xt, yt;
            do
            {
                xt = -std::log(uniform()) / r0;
                yt = -std::log(uniform());
            } while (yt + yt < xt * xt);
            return hz > 0 ? r0 + xt : -r0 - xt;
        }
        if (fn[iz] + uniform() * (fn[iz - 1] - fn[iz]) < std::exp(-0.5 * x * x))
            return x;
    }
}

// ============================================================================
// Settings
// ============================================================================
Bool_t R3BGTPCVMMDigitizer::CheckSettings() const
{
    Bool_t ok = kTRUE;
    auto fail = [&ok](const char* msg)
    {
        LOG(error) << "R3BGTPCVMMDigitizer: " << msg;
        ok = kFALSE;
    };

    if (!(fDetectorGain > 0.))
        fail("SetDetectorGain(G) with G > 0 is required");
    if (fUsePolya && !(fPolyaTheta >= 0.))
        fail("Polya theta must be >= 0");
    if (std::find(kVMMGains.begin(), kVMMGains.end(), fVMMGainMVPerFC) == kVMMGains.end())
        fail("SetVMMGain: allowed 0.5, 1, 3, 4.5, 6, 9, 12, 16 mV/fC");
    if (std::find(kVMMPeakingTimes.begin(), kVMMPeakingTimes.end(), fPeakingTimeNs) == kVMMPeakingTimes.end())
        fail("SetPeakingTime: allowed 25, 50, 100, 200 ns");
    if (!(fDeadTimeNs > 0.))
        fail("SetDeadTime(ns) > 0 is required: after the peak, VMM3a 220 ns digitization + 30 ns reset = 250 ns");
    if (fResetTimeNs > fDeadTimeNs)
        fail("SetResetTime(ns) must not be larger than the dead time (VMM3a: 30 of 250 ns)");
    if (!(fResetTimeNs >= 0.))
        fail("SetResetTime(ns) >= 0 is required (VMM3a: 30 ns)");
    if (!(fADCCountsPerMV > 0.))
        fail("SetADCCountsPerMV with value > 0 is required");
    if (!(fENCElectrons > 0.))
        fail("SetENC(electrons rms) with value > 0 is required");
    if (!(fThresholdSigmaInput > 0.) && !(fThresholdMVInput > 0.))
        fail("set the threshold: SetThresholdSigma(n) or SetThresholdMV(THL)");
    if (fDLCEnabled && !(fDLCRCInput > 0.) && !(fDLCOhmPerSquare > 0. && fDLCInsulatorUm > 0. && fDLCEpsR >= 1.))
        fail("SetDLC(R [ohm/square] > 0, insulator [um] > 0, epsR >= 1) or SetDLCRC(ns/mm^2 > 0)");
    if (fDLCEnabled && !(fDLCSigma0Mm >= 0.))
        fail("SetDLCInitialSigma(mm) needs a value >= 0");
    return ok;
}

// T(s) = 1 / ((s + a)((s + r)^2 + c^2)) in units of alpha [VMM, eq. 2-8]. Response to a
// point charge: h(x) = K1 exp(-a x) + 2 Re[K2 exp((-r + i c) x)], x = t / alpha.
// alpha is chosen so that h peaks at the peaking time; K1, K2 scaled to peak height 1.
void R3BGTPCVMMDigitizer::PrepareShaper()
{
    const Double_t a = kPoleReal, r = kPoleRe, c = kPoleIm;
    const Double_t K1 = 1. / ((r - a) * (r - a) + c * c);
    const std::complex<Double_t> K2 = 1. / (std::complex<Double_t>(0., 2. * c) * std::complex<Double_t>(a - r, c));
    const std::complex<Double_t> lambda(-r, c);
    auto response = [&](Double_t x) { return K1 * std::exp(-a * x) + 2. * std::real(K2 * std::exp(lambda * x)); };

    // golden-section search for the maximum (x = 1.48, [VMM] quotes t_peak = 1.5 alpha)
    Double_t lo = 0.5, hi = 3.;
    const Double_t golden = 0.5 * (std::sqrt(5.) - 1.);
    for (Int_t i = 0; i < 200; ++i)
    {
        const Double_t x1 = hi - golden * (hi - lo);
        const Double_t x2 = lo + golden * (hi - lo);
        (response(x1) < response(x2)) ? lo = x1 : hi = x2;
    }
    const Double_t xPeak = 0.5 * (lo + hi);
    const Double_t peakHeight = response(xPeak);

    const Double_t alpha = fPeakingTimeNs / xPeak; // [ns]
    fRealRate = a / alpha;
    fComplexRate = lambda / alpha;
    fK1 = K1 / peakHeight;
    fK2 = K2 / peakHeight;
    fStepNs = fPeakingTimeNs / kStepsPerPeakingTime;
    // constant input current I during one grid step: x' = -a x + I, z' = l z + I
    fCurrentRealGain = (1. - std::exp(-fRealRate * fStepNs)) / fRealRate;
    fCurrentComplexGain = (std::exp(fComplexRate * fStepNs) - 1.) / fComplexRate;
}

// ============================================================================
// Shaper helpers
// ============================================================================
Double_t R3BGTPCVMMDigitizer::ShaperOutput(Double_t realState, const std::complex<Double_t>& complexState) const
{
    return fK1 * realState + 2. * std::real(fK2 * complexState);
}

Double_t R3BGTPCVMMDigitizer::ShaperSlope(Double_t realState, const std::complex<Double_t>& complexState) const
{
    return -fRealRate * fK1 * realState + 2. * std::real(fK2 * fComplexRate * complexState);
}

void R3BGTPCVMMDigitizer::MoveForward(Double_t& realState, std::complex<Double_t>& complexState, Double_t dt) const
{
    realState *= std::exp(-fRealRate * dt);
    complexState *= std::exp(fComplexRate * dt);
}

// Rice R0 (arXiv:1305.4592, eq. 3.6) for white series noise through this shaper:
//   W(w) ~ w^2 |H(iw)|^2,   R0 = (1 / 2 pi) sqrt( Int w^2 W dw / Int W dw )      [1/ns]
// Integrated on a logarithmic grid; above w_max the integrands fall as w^-4 and w^-2
// (|H|^2 -> w^-6), their tails are added analytically.
Double_t R3BGTPCVMMDigitizer::ShaperNoiseR0PerNs() const
{
    const Double_t a = fRealRate;
    auto absH2 = [&](Double_t w)
    {
        const std::complex<Double_t> s(0., w);
        return 1. / std::norm((s + a) * (s - fComplexRate) * (s - std::conj(fComplexRate)));
    };
    const Double_t wScale = std::max(a, std::abs(fComplexRate));
    const Double_t wMin = 1.e-6 * wScale, wMax = 1.e4 * wScale;
    const Int_t n = 200000;
    const Double_t dl = std::log(wMax / wMin) / n;
    Double_t m0 = 0., m2 = 0.;
    for (Int_t i = 0; i < n; ++i)
    {
        const Double_t w = wMin * std::exp((i + 0.5) * dl);
        const Double_t W = w * w * absH2(w);
        m0 += W * w * dl;
        m2 += w * w * W * w * dl;
    }
    m0 += 1. / (3. * wMax * wMax * wMax);
    m2 += 1. / wMax;
    return std::sqrt(m2 / m0) / kTwoPi;
}

// ============================================================================
// Parameters: GTPCGasPar (drift velocity), as in the projector
// ============================================================================
void R3BGTPCVMMDigitizer::SetParContainers()
{
    FairRunAna* run = FairRunAna::Instance();
    FairRuntimeDb* rtdb = run ? run->GetRuntimeDb() : nullptr;
    if (rtdb)
        fGasPar = dynamic_cast<R3BGTPCGasPar*>(rtdb->getContainer("GTPCGasPar"));
}

// ============================================================================
// Init
// ============================================================================
InitStatus R3BGTPCVMMDigitizer::Init()
{
    auto* manager = FairRootManager::Instance();
    if (!manager)
    {
        LOG(error) << "R3BGTPCVMMDigitizer: no FairRootManager";
        return kFATAL;
    }
    fProjPoints = dynamic_cast<TClonesArray*>(manager->GetObject("GTPCProjPoint"));
    fMapInput = dynamic_cast<TClonesArray*>(manager->GetObject("GTPCMap"));
    if (!fProjPoints || !fMapInput)
    {
        LOG(error) << "R3BGTPCVMMDigitizer: GTPCProjPoint / GTPCMap branch missing "
                   << "(run the projector with SetProjPointsAsOutput())";
        return kFATAL;
    }
    if (!CheckSettings())
        return kFATAL;

    PrepareShaper();

    // ---- the one conversion into ADC counts
    fADCPerElectron = kElectronChargeFC * fVMMGainMVPerFC * fADCCountsPerMV;
    fNoiseSigmaADC = fENCElectrons * fADCPerElectron;
    if (fNoiseEnabled)
        PrepareNoise();
    fThresholdADC =
        fThresholdMVInput > 0. ? fThresholdMVInput * fADCCountsPerMV : fThresholdSigmaInput * fNoiseSigmaADC;
    fThresholdSigma = fThresholdADC / fNoiseSigmaADC;

    // ---- resistive layer: R [ohm] * C [F/mm^2] = s/mm^2 -> ns/mm^2; inside the class cm and ns
    if (fDLCEnabled)
    {
        fDLCRCnsPerMm2 = fDLCRCInput > 0.
                             ? fDLCRCInput
                             : fDLCOhmPerSquare * kEps0FPerM * fDLCEpsR / (fDLCInsulatorUm * 1.e-6) * 1.e-6 * 1.e9;
        fDLCRCnsPerCm2 = 100. * fDLCRCnsPerMm2; // 1 cm^2 = 100 mm^2
        fDLCSigma0Cm = 0.1 * fDLCSigma0Mm;      // 1 mm = 0.1 cm
    }

    // ---- Rice rate of noise-only hits
    fNoiseKUsed = fNoiseKFilter > 0. ? fNoiseKFilter : ShaperNoiseR0PerNs() * fPeakingTimeNs;
    fNoiseR0PerNs = fNoiseKUsed / fPeakingTimeNs;
    fNoiseRatePerNs = fNoiseR0PerNs * std::exp(-0.5 * fThresholdSigma * fThresholdSigma);
    // ---- peak-time jitter from the noise: near the peak the pulse is A [1 - (t - t_p)^2 c / 2],
    //      c = |h''(t_p)| / h(t_p); the noise slope there has the rms sigma 2 pi R0 (Rice), so the
    //      maximum moves by sigma_t = sigma 2 pi R0 / (A c) = fPeakJitterNs sigma / A.
    //      Not used in the simulation (the noise process moves the peak by itself); written to
    //      the run settings for the hit errors of the reconstruction (time at the peak).
    {
        const Double_t h = ResponseAt(fPeakingTimeNs), e = 1.e-3 * fPeakingTimeNs;
        const Double_t curvature =
            std::abs(ResponseAt(fPeakingTimeNs + e) - 2. * h + ResponseAt(fPeakingTimeNs - e)) / (e * e * h);
        fPeakJitterNs = 2. * TMath::Pi() * ShaperNoiseR0PerNs() / curvature;
    }
    // ---- readout window of one event = full drift time: drift length (Active_region in the
    //      TGeo geometry) / drift velocity (GTPCGasPar). Only needed for the noise hits.
    if (fNoiseEnabled)
    {
        const Double_t v = fGasPar ? fGasPar->GetDriftVelocity() : -1.;
        Double_t driftLength = -1.;
        if (gGeoManager && gGeoManager->GetTopVolume())
        {
            TGeoIterator it(gGeoManager->GetTopVolume());
            while (TGeoNode* node = it())
                if (node->GetVolume() && TString(node->GetVolume()->GetName()) == "Active_region")
                {
                    if (const auto* box = dynamic_cast<const TGeoBBox*>(node->GetVolume()->GetShape()))
                        driftLength = 2. * box->GetDY();
                    break;
                }
        }
        if (!(v > 0.) || !(driftLength > 0.))
        {
            LOG(error) << "R3BGTPCVMMDigitizer: the noise needs the readout window = drift length / drift velocity: "
                       << "load the geometry (run->SetGeomFile) and GTPCGasPar (the projector's .par file as "
                       << "second parameter input)";
            return kFATAL;
        }
        fReadoutWindowNs = driftLength / v;
    }

    fDigis = new TClonesArray("R3BGTPCVMMDigi");
    manager->Register("GTPCVMMDigi", "GTPC", fDigis, fPersistence);
    fDigiMatches = new TClonesArray("R3BGTPCMatch");
    manager->Register("GTPCVMMDigiMatch", "GTPC", fDigiMatches, fPersistence);

    fRng.seed(fRandomSeed);
    fNoiseRng.seed(fRandomSeed + 1);

    LOG(info) << "R3BGTPCVMMDigitizer (all amplitudes in ADC counts):";
    LOG(info) << "  detector gain     = " << fDetectorGain
              << (fUsePolya ? " (Polya, theta = " + std::to_string(fPolyaTheta) + ")" : " (no fluctuation)") << " -> "
              << GetADCPerDriftElectron() << " ADC per drift electron";
    LOG(info) << "  VMM3a             = " << fVMMGainMVPerFC << " mV/fC, peaking time " << fPeakingTimeNs << " ns, "
              << fADCCountsPerMV << " ADC/mV";
    LOG(info) << "  dead time         = from the rising edge (threshold crossing) to the peak + " << fDeadTimeNs
              << " ns (" << fDeadTimeNs - fResetTimeNs << " ns digitization + " << fResetTimeNs
              << " ns reset), or the falling edge + reset if later; a new hit "
              << "needs a new rising edge (VMM3 spec. App. A, Figs. 21 and 23)";
    LOG(info) << "  noise sigma       = " << fNoiseSigmaADC << " ADC (ENC " << fENCElectrons << " e)";
    LOG(info) << "  threshold         = " << fThresholdADC << " ADC = " << fThresholdSigma
              << " sigma = " << fThresholdADC / fADCCountsPerMV << " mV above baseline";
    if (fNoiseEnabled)
    {
        LOG(info) << "  noise             = ON: Gaussian process in time on the pads with charge (+ neighbours); Rice "
                     "noise hits elsewhere, K_filter = "
                  << fNoiseKUsed << (fNoiseKFilter > 0. ? " (set)" : " (VMM3a shaper, series noise)")
                  << ", R0 = " << fNoiseR0PerNs * 1.e3 << " MHz, rate " << fNoiseRatePerNs * 1.e9 << " Hz per channel";
        LOG(info) << "                      readout window " << fReadoutWindowNs << " ns (drift length / v) -> "
                  << fNoiseRatePerNs * fReadoutWindowNs << " noise hits per pad and event";
        if (fThresholdSigma < 2.5)
            LOG(warn) << "R3BGTPCVMMDigitizer: threshold " << fThresholdSigma
                      << " sigma: below ~2.5 sigma noise crossings are not independent, the Rice / Poisson "
                         "noise-hit model becomes approximate";
    }
    else
        LOG(info) << "  noise             = off (noiseless electronics)";
    if (fDLCEnabled)
    {
        LOG(info) << "  DLC               = ON: RC = " << fDLCRCnsPerMm2 << " ns/mm^2"
                  << (fDLCRCInput > 0.
                          ? std::string(" (set)")
                          : " (R = " + std::to_string(fDLCOhmPerSquare) + " ohm/sq, insulator " +
                                std::to_string(fDLCInsulatorUm) + " um, epsR " + std::to_string(fDLCEpsR) + ")")
                  << ", sigma0 = " << fDLCSigma0Mm << " mm";
        LOG(info) << "                      spread at the peaking time: sigma(t_p) = "
                  << 10. * DLCSigmaCm(fPeakingTimeNs) << " mm, at 2 t_p: " << 10. * DLCSigmaCm(2. * fPeakingTimeNs)
                  << " mm";
    }
    else
        LOG(info) << "  DLC               = off (all charge on the pad of the landing point)";
    LOG(info) << "  timing            = " << (fTimeAtThreshold ? "at the threshold crossing" : "at the peak")
              << " (VMM3a register)";
    LOG(info) << "  neighbour logic   = "
              << (fNeighborRows || fNeighborColumns ? std::string(fNeighborRows ? "rows (row +- 1)" : "") +
                                                          (fNeighborRows && fNeighborColumns ? " and " : "") +
                                                          (fNeighborColumns ? "columns (column +- 1)" : "")
                                                    : std::string("off"));
    return kSUCCESS;
}

// The pad map is only filled once an event has been read, so this runs in the first Exec().
Bool_t R3BGTPCVMMDigitizer::ReadPadGeometry()
{
    const auto* map = fMapInput->GetEntriesFast() > 0 ? dynamic_cast<const R3BGTPCMap*>(fMapInput->At(0)) : nullptr;
    if (!map || map->GetNumberOfPads() <= 0)
    {
        LOG(error) << "R3BGTPCVMMDigitizer: invalid GTPCMap entry";
        return kFALSE;
    }
    fNRows = map->GetNumberOfRowsX();
    fNColumns = map->GetNumberOfColumnsZ();
    fPitchXcm = map->GetPitchXcm();
    fPitchZcm = map->GetPitchZcm();
    fGapXcm = map->GetGapXcm();
    fGapZcm = map->GetGapZcm();
    fOffsetXcm = map->GetGridOffsetXcm();
    fOffsetZcm = map->GetGridOffsetZcm();
    if (fDLCEnabled)
        PrepareDLC();
    LOG(info) << "R3BGTPCVMMDigitizer: " << fNRows << " rows x " << fNColumns << " columns = " << GetNumberOfPads()
              << " channels";
    fGeometryRead = kTRUE;
    return kTRUE;
}

// ============================================================================
// Charge input
// ============================================================================
Double_t R3BGTPCVMMDigitizer::AvalancheADC()
{
    Double_t nElectrons = fDetectorGain;
    if (fUsePolya)
    {
        std::gamma_distribution<Double_t> polya(1. + fPolyaTheta, fDetectorGain / (1. + fPolyaTheta));
        nElectrons = polya(fRng);
    }
    return nElectrons * fADCPerElectron;
}

// A point charge on a pad at timeNs, stored in the grid step that ends right after it,
// already moved to the end of that step.
void R3BGTPCVMMDigitizer::AddCharge(Int_t pad, Double_t timeNs, Double_t chargeADC)
{
    AddChargeTo(fPads[pad], timeNs, chargeADC);
}

void R3BGTPCVMMDigitizer::AddChargeTo(PadInput& input, Double_t timeNs, Double_t chargeADC)
{
    const Long64_t step = static_cast<Long64_t>(std::ceil(timeNs / fStepNs));
    const Double_t timeToStepEnd = static_cast<Double_t>(step) * fStepNs - timeNs;
    GridStep& gridStep = input.At(step);
    gridStep.realState += chargeADC * std::exp(-fRealRate * timeToStepEnd);
    gridStep.complexState += chargeADC * std::exp(fComplexRate * timeToStepEnd);
}

// ============================================================================
// Resistive layer (DLC): charge spreading [Dixit & Rankin, NIM A 566 (2006) 281]
//
// Units inside: lengths in cm, times in ns, charges in ADC. RC [ns/cm^2] = 100 x RC [ns/mm^2].
//
// A deposit of charge Q (one avalanche, or several of the same pad cell, sub-cell bin, grid step
// and MC track) landing at (x0, z0) at t0 is a 2D Gaussian on the DLC,
//     sigma^2(tau) = sigma0^2 + 2 tau / RC,   tau = t - t0,
// and pad p holds Q_p(tau) = Q Fx(tau) Fz(tau) (the Gaussian over the pad cell). Shaper input of pad p:
// Q_p(0) at t0, the change during the first (partial) grid step as a point charge in its middle,
// then constant currents (Q_p(tau_n) - Q_p(tau_n-1)) / (tau_n - tau_n-1) between the time nodes.
// After the last node the charge left on a pad stays (its slow later decrease is neglected). Charge
// that spreads beyond the outermost pads is lost.
//
// Fast evaluation: the nodes after tau = 0 are grid points, tau_n = phi + m_n step with
// phi = first grid point after t0 - t0 in (0, step]. Fx and Fz depend only on the landing point
// within its pad cell, phi and the pad distance, so they are tabulated once (PrepareDLC) for
// kDLCOffsetBins positions and kDLCPhaseBins phases; an event needs no error function at all.
// ============================================================================
Double_t R3BGTPCVMMDigitizer::DLCSigmaCm(Double_t tauNs) const
{
    return std::sqrt(fDLCSigma0Cm * fDLCSigma0Cm + 2. * std::max(0., tauNs) / fDLCRCnsPerCm2);
}

void R3BGTPCVMMDigitizer::PrepareDLC()
{
    // ---- how long and how far the spreading is followed
    const Double_t sigmaMaxCm = kDLCMaxSigmaInPitch * std::max(fPitchXcm, fPitchZcm);
    const Double_t tauSigmaMax =
        0.5 * fDLCRCnsPerCm2 * std::max(0., sigmaMaxCm * sigmaMaxCm - fDLCSigma0Cm * fDLCSigma0Cm);
    fDLCMaxTimeNs = std::max(fStepNs, std::min(tauSigmaMax, kDLCMaxPeakingTimes * fPeakingTimeNs));
    const Double_t reachCm = kDLCWindowNSigma * DLCSigmaCm(fDLCMaxTimeNs);
    fDLCReachRows = static_cast<Int_t>(std::ceil(reachCm / fPitchXcm)) + 1;
    fDLCReachColumns = static_cast<Int_t>(std::ceil(reachCm / fPitchZcm)) + 1;

    // ---- node offsets m_n [grid steps] after the first grid point: every step while 15 % of tau is
    //      less than a step, then +15 %
    fDLCNodeSteps.assign(1, 0);
    while (static_cast<Double_t>(fDLCNodeSteps.back()) * fStepNs < fDLCMaxTimeNs)
    {
        const Double_t tau = (static_cast<Double_t>(fDLCNodeSteps.back()) + 0.5) * fStepNs;
        fDLCNodeSteps.push_back(fDLCNodeSteps.back() +
                                std::max<Long64_t>(1, std::llround(kDLCStepGrowth * tau / fStepNs)));
    }
    fDLCNodes = static_cast<Int_t>(fDLCNodeSteps.size()) + 1; // node 0: tau = 0; node n >= 1: tau = phi + m_{n-1} step

    // ---- tables of the pad shares, per axis: [phase][node][offset bin][pad distance + reach]
    auto fill = [&](Double_t pitchCm, Int_t reach, std::vector<Float_t>& table, std::vector<Float_t>& maxTable)
    {
        const Int_t nRel = 2 * reach + 1;
        constexpr Int_t nU = kDLCOffsetBins + 1; // landing points u = b / kDLCOffsetBins, b = 0 ... kDLCOffsetBins
        table.assign(static_cast<std::size_t>(kDLCPhaseBins) * fDLCNodes * nU * nRel, 0.f);
        maxTable.assign(static_cast<std::size_t>(kDLCPhaseBins) * kDLCOffsetBins * nRel, 0.f);
        for (Int_t ph = 0; ph < kDLCPhaseBins; ++ph)
        {
            const Double_t phi = (ph + 0.5) / kDLCPhaseBins * fStepNs;
            for (Int_t n = 0; n < fDLCNodes; ++n)
            {
                const Double_t tau = n == 0 ? 0. : phi + static_cast<Double_t>(fDLCNodeSteps[n - 1]) * fStepNs;
                const Double_t sigma = DLCSigmaCm(tau);
                for (Int_t b = 0; b < nU; ++b)
                {
                    const Double_t u =
                        static_cast<Double_t>(b) / kDLCOffsetBins; // landing point / pitch from the low cell edge
                    for (Int_t r = -reach; r <= reach; ++r)
                    {
                        const Double_t lo = (r - u) * pitchCm; // cell r relative to the landing point
                        const Float_t f = static_cast<Float_t>(FractionInside(0., sigma, lo, lo + pitchCm));
                        table[((static_cast<std::size_t>(ph) * fDLCNodes + n) * nU + b) * nRel + (r + reach)] = f;
                    }
                }
            }
        }
        // largest share of every pad over all nodes, per landing bin (both ends of the bin)
        for (Int_t ph = 0; ph < kDLCPhaseBins; ++ph)
            for (Int_t n = 0; n < fDLCNodes; ++n)
                for (Int_t b = 0; b < kDLCOffsetBins; ++b)
                    for (Int_t r = 0; r < nRel; ++r)
                    {
                        const std::size_t base = (static_cast<std::size_t>(ph) * fDLCNodes + n) * nU;
                        Float_t& m = maxTable[(static_cast<std::size_t>(ph) * kDLCOffsetBins + b) * nRel + r];
                        m = std::max({ m, table[(base + b) * nRel + r], table[(base + b + 1) * nRel + r] });
                    }
    };
    fill(fPitchXcm, fDLCReachRows, fDLCTableX, fDLCMaxX);
    fill(fPitchZcm, fDLCReachColumns, fDLCTableZ, fDLCMaxZ);

    LOG(info) << "R3BGTPCVMMDigitizer: DLC spreading followed for " << fDLCMaxTimeNs
              << " ns (sigma = " << 10. * DLCSigmaCm(fDLCMaxTimeNs) << " mm) in " << fDLCNodes
              << " time nodes, up to +- " << fDLCReachRows << " rows and +- " << fDLCReachColumns
              << " columns from the landing pad; pad shares tabulated (" << kDLCOffsetBins << " x " << kDLCOffsetBins
              << " landing positions per pad, " << kDLCPhaseBins << " arrival phases per " << fStepNs << " ns step)";
}

// One drift electron: its pad cell, landing-point bins, grid step and MC track; electrons with the
// same values are one deposit (charge added, arrival time charge-weighted)
void R3BGTPCVMMDigitizer::CollectDLCElectron(Double_t xCm,
                                             Double_t zCm,
                                             Double_t timeNs,
                                             Double_t chargeADC,
                                             Int_t mcTrack)
{
    const Double_t ux = (xCm - fOffsetXcm + 0.5 * fGapXcm) / fPitchXcm;
    const Double_t uz = (zCm - fOffsetZcm + 0.5 * fGapZcm) / fPitchZcm;
    const Int_t row = static_cast<Int_t>(std::floor(ux)), col = static_cast<Int_t>(std::floor(uz));
    const Int_t bx = std::clamp(static_cast<Int_t>((ux - row) * kDLCOffsetBins), 0, kDLCOffsetBins - 1);
    const Int_t bz = std::clamp(static_cast<Int_t>((uz - col) * kDLCOffsetBins), 0, kDLCOffsetBins - 1);
    const Long64_t firstStep = static_cast<Long64_t>(std::floor(timeNs / fStepNs)) + 1;
    const Double_t phi = static_cast<Double_t>(firstStep) * fStepNs - timeNs;
    const Int_t ph = std::clamp(static_cast<Int_t>(phi / fStepNs * kDLCPhaseBins), 0, kDLCPhaseBins - 1);
    DLCDepositKey key{ row, col, bx, bz, ph, firstStep, mcTrack };
    DLCDeposit& d = fDLCDeposits[key];
    d.chargeADC += chargeADC;
    d.chargeTimeADCns += chargeADC * timeNs;
    d.chargeUx += chargeADC * (ux - row); // landing point within the cell, 0 ... 1
    d.chargeUz += chargeADC * (uz - col);
}

std::size_t R3BGTPCVMMDigitizer::DLCDepositKeyHash::operator()(const DLCDepositKey& k) const
{
    std::size_t h = std::hash<Long64_t>()(k.firstStep);
    for (const Long64_t v : { static_cast<Long64_t>(k.row),
                              static_cast<Long64_t>(k.col),
                              static_cast<Long64_t>(k.bx),
                              static_cast<Long64_t>(k.bz),
                              static_cast<Long64_t>(k.phase),
                              static_cast<Long64_t>(k.mcTrack) })
        h ^= std::hash<Long64_t>()(v) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

// Charge spreading of every deposit of the event onto the pads (shaper inputs per pad and MC track)
void R3BGTPCVMMDigitizer::SpreadDLCDeposits()
{
    // fixed order of the deposits (reproducible sums)
    std::vector<std::pair<DLCDepositKey, DLCDeposit>> deposits(fDLCDeposits.begin(), fDLCDeposits.end());
    std::sort(deposits.begin(),
              deposits.end(),
              [](const auto& a, const auto& b)
              {
                  const DLCDepositKey &x = a.first, &y = b.first;
                  return std::tie(x.firstStep, x.row, x.col, x.bx, x.bz, x.phase, x.mcTrack) <
                         std::tie(y.firstStep, y.row, y.col, y.bx, y.bz, y.phase, y.mcTrack);
              });
    const Int_t nRelX = 2 * fDLCReachRows + 1, nRelZ = 2 * fDLCReachColumns + 1;
    std::vector<Double_t> q(fDLCNodes);
    for (const auto& [key, dep] : deposits)
    {
        const Double_t Q = dep.chargeADC;
        if (!(Q > 0.))
            continue;
        const Double_t t0 = dep.chargeTimeADCns / Q;
        const Double_t phi = static_cast<Double_t>(key.firstStep) * fStepNs - t0; // (0, step]
        // landing point within the cell (charge-weighted), interpolated between the table points b, b + 1
        constexpr Int_t nU = kDLCOffsetBins + 1;
        const Double_t fx = std::clamp(dep.chargeUx / Q * kDLCOffsetBins - key.bx, 0., 1.);
        const Double_t fz = std::clamp(dep.chargeUz / Q * kDLCOffsetBins - key.bz, 0., 1.);
        const std::size_t nodeStrideX = static_cast<std::size_t>(nU) * nRelX,
                          nodeStrideZ = static_cast<std::size_t>(nU) * nRelZ;
        const Float_t* tableX = &fDLCTableX[(static_cast<std::size_t>(key.phase) * fDLCNodes * nU + key.bx) * nRelX];
        const Float_t* tableZ = &fDLCTableZ[(static_cast<std::size_t>(key.phase) * fDLCNodes * nU + key.bz) * nRelZ];
        const Float_t* maxX = &fDLCMaxX[(static_cast<std::size_t>(key.phase) * kDLCOffsetBins + key.bx) * nRelX];
        const Float_t* maxZ = &fDLCMaxZ[(static_cast<std::size_t>(key.phase) * kDLCOffsetBins + key.bz) * nRelZ];
        auto shareX = [&](Int_t n, Int_t ix)
        {
            const Float_t* t = tableX + n * nodeStrideX + ix;
            return t[0] + fx * (t[nRelX] - t[0]);
        };
        auto shareZ = [&](Int_t n, Int_t jz)
        {
            const Float_t* t = tableZ + n * nodeStrideZ + jz;
            return t[0] + fz * (t[nRelZ] - t[0]);
        };

        for (Int_t j = -fDLCReachColumns; j <= fDLCReachColumns; ++j)
        {
            const Int_t col = key.col + j;
            if (col < 0 || col >= fNColumns)
                continue;
            const Int_t jz = j + fDLCReachColumns;
            for (Int_t i = -fDLCReachRows; i <= fDLCReachRows; ++i)
            {
                const Int_t row = key.row + i;
                const Int_t ix = i + fDLCReachRows;
                if (row < 0 || row >= fNRows || maxX[ix] * maxZ[jz] < kDLCMinFraction)
                    continue;
                for (Int_t n = 0; n < fDLCNodes; ++n)
                    q[n] = Q * shareX(n, ix) * shareZ(n, jz);
                PadInput& input = fTrackInputs[col * fNRows + row][key.mcTrack];
                if (q[0] != 0.)
                    AddChargeTo(input, t0, q[0]);
                if (fDLCNodes > 1 && q[1] != q[0])
                    AddChargeTo(input, t0 + 0.5 * phi, q[1] - q[0]);
                if (fDLCNodes <= 2)
                    continue;
                // constant current between node n-1 (grid step firstStep + m_{n-2}) and node n
                // (firstStep + m_{n-1}); one change of the current per node. The steps of all nodes are
                // allocated once, then addressed directly.
                input.At(key.firstStep + fDLCNodeSteps[fDLCNodes - 2]);
                input.At(key.firstStep);
                GridStep* base = &input.steps[static_cast<std::size_t>(key.firstStep - input.firstStep)];
                Double_t previous = 0.;
                for (Int_t n = 2; n < fDLCNodes; ++n)
                {
                    const Long64_t mA = fDLCNodeSteps[n - 2], mB = fDLCNodeSteps[n - 1];
                    const Double_t current = (q[n] - q[n - 1]) / (static_cast<Double_t>(mB - mA) * fStepNs);
                    GridStep& g = base[mA];
                    g.currentChange += current - previous;
                    g.hasInput = kTRUE;
                    previous = current;
                }
                GridStep& gEnd = base[fDLCNodeSteps[fDLCNodes - 2]];
                gEnd.currentChange -= previous;
                gEnd.hasInput = kTRUE;
            }
        }
    }
}

// Noiseless signal [ADC] of one shaper input on the grid points kFirst ... kLast (as in DigitizePad)
void R3BGTPCVMMDigitizer::SignalOnGrid(const PadInput& input,
                                       Long64_t kFirst,
                                       Long64_t kLast,
                                       std::vector<Double_t>& out) const
{
    out.assign(static_cast<std::size_t>(std::max<Long64_t>(0, kLast - kFirst + 1)), 0.);
    if (input.Empty() || out.empty())
        return;
    const Long64_t lastStep = input.usedLast;
    const Double_t realDecay = std::exp(-fRealRate * fStepNs);
    const std::complex<Double_t> complexDecay = std::exp(fComplexRate * fStepNs);
    Double_t realState = 0., current = 0.;
    std::complex<Double_t> complexState = 0.;
    for (Long64_t k = input.usedFirst; k <= kLast; ++k)
    {
        realState *= realDecay;
        complexState *= complexDecay;
        if (current != 0.)
        {
            realState += current * fCurrentRealGain;
            complexState += current * fCurrentComplexGain;
        }
        if (k <= lastStep)
        {
            const GridStep& gridStep = input.steps[static_cast<std::size_t>(k - input.firstStep)];
            realState += gridStep.realState;
            complexState += gridStep.complexState;
            current += gridStep.currentChange;
        }
        if (k >= lastStep)
            current = 0.;
        if (k >= kFirst)
            out[static_cast<std::size_t>(k - kFirst)] = ShaperOutput(realState, complexState);
    }
}

// ============================================================================
// Noise of one pulse: n ~ Gauss(0, sigma). Values that would put the effective threshold
// u - n below one sigma are cut: there the noise ALONE crosses the threshold, and those
// hits are the Rice noise hits (GenerateNoise), not a property of the pulse.
// ============================================================================
// ============================================================================
// Electronics noise as a process in time. White series noise at the amplifier input,
// shaped like the signal: its spectrum at the shaper output is w^2 |H(iw)|^2, i.e. white
// noise through H(s) s, impulse response h'(t) = K1 l1 e^{l1 t} + 2 Re(K2 l2 e^{l2 t}).
// With the states x1 = int e^{l1 (t-s)} w ds and z = int e^{l2 (t-s)} w ds (z complex) the
// process is generated EXACTLY on the time grid: X_{n+1} = Phi X_n + eps,
// eps ~ N(0, Q), Q = int_0^dt v v^T, v(s) = (e^{l1 s}, Re e^{l2 s}, Im e^{l2 s}); the start
// value is drawn from the stationary covariance P = int_0^inf v v^T. Output n = c . X,
// scaled to rms = ENC. Its autocorrelation and up-crossing rate follow from the shaper: the
// up-crossing rate is the Rice rate R0 exp(-u^2 / 2 sigma^2) with R0 = 0.519 / t_p.
// ============================================================================
void R3BGTPCVMMDigitizer::PrepareNoise()
{
    const Double_t l1 = -fRealRate;                 // real pole [1/ns]
    const std::complex<Double_t> l2 = fComplexRate; // complex pole [1/ns]
    const Double_t dt = fStepNs;
    const std::complex<Double_t> e2 = std::exp(l2 * dt);
    const Double_t e1 = std::exp(l1 * dt);
    const Double_t phi[3][3] = { { e1, 0., 0. }, { 0., e2.real(), -e2.imag() }, { 0., e2.imag(), e2.real() } };
    for (Int_t i = 0; i < 3; ++i)
        for (Int_t j = 0; j < 3; ++j)
            fNoisePhi[i][j] = phi[i][j];

    // int_0^T v v^T ds (Simpson)
    auto covariance = [&](Double_t T, Double_t M[3][3])
    {
        const Int_t n = 20000; // even
        for (Int_t i = 0; i < 3; ++i)
            for (Int_t j = 0; j < 3; ++j)
                M[i][j] = 0.;
        for (Int_t k = 0; k <= n; ++k)
        {
            const Double_t sk = T * k / n;
            const Double_t w = (k == 0 || k == n ? 1. : (k % 2 ? 4. : 2.)) * T / (3. * n);
            const std::complex<Double_t> z = std::exp(l2 * sk);
            const Double_t v[3] = { std::exp(l1 * sk), z.real(), z.imag() };
            for (Int_t i = 0; i < 3; ++i)
                for (Int_t j = 0; j < 3; ++j)
                    M[i][j] += w * v[i] * v[j];
        }
    };
    // lower Cholesky factor of a positive semi-definite 3x3 matrix
    auto cholesky = [](Double_t M[3][3], Double_t L[3][3])
    {
        for (Int_t i = 0; i < 3; ++i)
            for (Int_t j = 0; j < 3; ++j)
                L[i][j] = 0.;
        for (Int_t j = 0; j < 3; ++j)
        {
            Double_t d = M[j][j];
            for (Int_t k = 0; k < j; ++k)
                d -= L[j][k] * L[j][k];
            L[j][j] = d > 0. ? std::sqrt(d) : 0.;
            for (Int_t i = j + 1; i < 3; ++i)
            {
                Double_t x = M[i][j];
                for (Int_t k = 0; k < j; ++k)
                    x -= L[i][k] * L[j][k];
                L[i][j] = L[j][j] > 0. ? x / L[j][j] : 0.;
            }
        }
    };
    Double_t Q[3][3], P[3][3];
    covariance(dt, Q);
    covariance(60. * kPoleReal / fRealRate, P); // 60 alpha: the states have decayed by e^-75
    cholesky(Q, fNoiseLQ);
    cholesky(P, fNoiseLP);

    const std::complex<Double_t> k2l = fK2 * l2;
    const Double_t c[3] = { fK1 * l1, 2. * k2l.real(), -2. * k2l.imag() };
    Double_t var = 0.;
    for (Int_t i = 0; i < 3; ++i)
        for (Int_t j = 0; j < 3; ++j)
            var += c[i] * P[i][j] * c[j];
    for (Int_t i = 0; i < 3; ++i)
        fNoiseC[i] = c[i] * fNoiseSigmaADC / std::sqrt(var);
}

// n consecutive noise values [ADC] on the time grid, starting from the stationary state
void R3BGTPCVMMDigitizer::NoiseSeries(std::size_t n, std::vector<Double_t>& out)
{
    out.assign(n, 0.);
    if (!fNoiseEnabled)
        return;
    const Ziggurat& gauss = fGauss;
    Double_t x[3], g[3];
    for (Int_t i = 0; i < 3; ++i)
        g[i] = gauss(fNoiseRng);
    for (Int_t i = 0; i < 3; ++i)
        x[i] = fNoiseLP[i][0] * g[0] + fNoiseLP[i][1] * g[1] + fNoiseLP[i][2] * g[2];
    for (std::size_t k = 0; k < n; ++k)
    {
        out[k] = fNoiseC[0] * x[0] + fNoiseC[1] * x[1] + fNoiseC[2] * x[2];
        for (Int_t i = 0; i < 3; ++i)
            g[i] = gauss(fNoiseRng);
        Double_t y[3];
        for (Int_t i = 0; i < 3; ++i)
            y[i] = fNoisePhi[i][0] * x[0] + fNoisePhi[i][1] * x[1] + fNoisePhi[i][2] * x[2] + fNoiseLQ[i][0] * g[0] +
                   fNoiseLQ[i][1] * g[1] + fNoiseLQ[i][2] * g[2];
        for (Int_t i = 0; i < 3; ++i)
            x[i] = y[i];
    }
}

// ============================================================================
// One channel with charge: the waveform signal + noise on the time grid, from 1.5 t_p before
// the first charge to 3 t_p after the last; then the chip logic on it (ChannelHits).
// The signal is exact on the grid points (linear shaper, charge moved to the grid point).
// ============================================================================
void R3BGTPCVMMDigitizer::DigitizePad(Int_t pad, PadInput& input, std::vector<Hit>& hits)
{
    if (input.Empty())
        return;
    const Long64_t lastStep = input.usedLast;
    const Long64_t kStart = input.usedFirst - static_cast<Long64_t>(std::ceil(kWindowBefore * kStepsPerPeakingTime));
    const Long64_t kEnd = lastStep + static_cast<Long64_t>(std::ceil(kWindowAfter * kStepsPerPeakingTime));
    Waveform& wf = fWaveforms[pad];
    wf.firstStep = kStart;
    NoiseSeries(static_cast<std::size_t>(kEnd - kStart + 1), wf.values);

    Double_t realState = 0.;
    std::complex<Double_t> complexState = 0.;
    Double_t current = 0.; // input current during the step (t_k-1, t_k] [ADC/ns] (DLC)
    const Double_t realDecay = std::exp(-fRealRate * fStepNs);
    const std::complex<Double_t> complexDecay = std::exp(fComplexRate * fStepNs);
    for (Long64_t k = kStart; k <= kEnd; ++k)
    {
        realState *= realDecay; // MoveForward over one grid step
        complexState *= complexDecay;
        if (current != 0.)
        {
            realState += current * fCurrentRealGain;
            complexState += current * fCurrentComplexGain;
        }
        if (k >= input.usedFirst && k <= lastStep)
        {
            const GridStep& gridStep = input.steps[static_cast<std::size_t>(k - input.firstStep)];
            realState += gridStep.realState;
            complexState += gridStep.complexState;
            current += gridStep.currentChange;
        }
        if (k >= lastStep)
            current = 0.; // all current changes applied: remove the rounding rest
        wf.values[static_cast<std::size_t>(k - kStart)] += ShaperOutput(realState, complexState);
    }
    ChannelHits(wf, hits);
}

// End of the dead time of a hit (VMM3 specifications v1.0, App. A, Figs. 21 and 23): the dead time
// starts at the rising edge; the 10-bit ADC digitizes the peak value from the peak on (220 ns), then the
// channel resets (30 ns). If the time over threshold is longer than the digitization, the last reset
// dominates: the reset follows the falling edge.
Double_t R3BGTPCVMMDigitizer::ReadyTime(Double_t peakNs, Double_t fallingEdgeNs) const
{
    return std::max(peakNs + fDeadTimeNs, fallingEdgeNs + fResetTimeNs);
}

// VMM3a logic on a sampled waveform: a hit starts where the waveform crosses the threshold
// from below (rising edge); the peak detector holds the first maximum after it; the channel is
// dead from the rising edge until ReadyTime (peak + digitization, or falling edge, + reset) and
// then needs a NEW rising edge: a signal that is still (or again) above threshold when the channel
// is ready again is not seen until it has gone below threshold and crosses it again.
// The waveform is the sum of all charges of the channel (signal + noise), so the cases of the
// specification (App. A, notes of clarification) follow from it:
//   - two signals further apart than the dead time: both detected;
//   - the second rises inside the dead time of the first (no overlap): lost;
//   - a third rises before the falling edge of that lost second: no new rising edge after the
//     channel is ready -> lost too (only the first is detected);
//   - overlapping signals (pile-up): one pulse above threshold from the first rising edge to the
//     last falling edge; the peak detector holds the first maximum (the sum if the second arrives
//     during the rise), and the reset follows the falling edge if that is after peak + digitization.
void R3BGTPCVMMDigitizer::ChannelHits(const Waveform& wf, std::vector<Hit>& hits) const
{
    const auto& v = wf.values;
    const Double_t u = fThresholdADC;
    auto timeOf = [&](std::size_t k)
    { return static_cast<Double_t>(wf.firstStep + static_cast<Long64_t>(k)) * fStepNs; };
    Double_t readyAt = -std::numeric_limits<Double_t>::infinity();
    for (std::size_t k = 1; k < v.size(); ++k)
    {
        if (!(v[k] >= u && v[k - 1] < u)) // rising edge between k - 1 and k
            continue;
        const Double_t tCross = timeOf(k) - fStepNs * (v[k] - u) / (v[k] - v[k - 1]);
        if (tCross < readyAt) // the channel is still dead (converting / resetting): edge not seen
            continue;
        std::size_t j = k;
        while (j + 1 < v.size() && v[j + 1] >= v[j])
            ++j;
        if (j + 1 >= v.size())
            break; // still rising at the end of the window
        // maximum between the samples: parabola through j - 1, j, j + 1
        const Double_t den = v[j - 1] - 2. * v[j] + v[j + 1];
        const Double_t dx = den != 0. ? 0.5 * (v[j - 1] - v[j + 1]) / den : 0.;
        const Double_t tPeak = (static_cast<Double_t>(wf.firstStep + static_cast<Long64_t>(j)) + dx) * fStepNs;
        const Double_t amplitude = v[j] - 0.25 * (v[j - 1] - v[j + 1]) * dx;
        // falling edge: first sample after the peak below threshold (interpolated); at the end of the
        // window if the signal does not go below threshold there (conservative: dead until then)
        Double_t tFall = timeOf(v.size() - 1);
        std::size_t m = j + 1;
        while (m < v.size() && v[m] >= u)
            ++m;
        if (m < v.size())
            tFall = timeOf(m) - fStepNs * (u - v[m]) / (v[m - 1] - v[m]);
        Hit hit;
        hit.crossingNs = tCross;
        hit.peakNs = tPeak;
        hit.amplitudeADC = amplitude;
        hit.noise = kFALSE;
        hit.readyNs = ReadyTime(tPeak, tFall);
        hits.push_back(hit);
        readyAt = hit.readyNs;
        k = j; // the next rising edge is searched after the peak
    }
}

// ============================================================================
// Noise-only hits (Rice formula), as CbmStsSimModule::GenerateNoise: Poisson number of
// hits with peak time in [t1, t2] for all channels together, random pad and time.
// ============================================================================
Int_t R3BGTPCVMMDigitizer::GenerateNoise(Double_t tStartNs, Double_t tEndNs)
{
    if (!(tEndNs > tStartNs))
        return 0;
    const Int_t nPads = GetNumberOfPads();
    const Double_t mean = fNoiseRatePerNs * nPads * (tEndNs - tStartNs);
    const Int_t nNoise = std::poisson_distribution<Int_t>(mean)(fNoiseRng);

    std::uniform_int_distribution<Int_t> anyPad(0, nPads - 1);
    std::uniform_real_distribution<Double_t> anyTime(tStartNs, tEndNs);
    std::uniform_real_distribution<Double_t> flat(0., 1.);
    const Double_t u = fThresholdADC;
    const Double_t sigma = fNoiseSigmaADC;
    const Double_t twoPiR0 = kTwoPi * fNoiseR0PerNs;

    for (Int_t i = 0; i < nNoise; ++i)
    {
        const Int_t pad = anyPad(fNoiseRng);
        const Double_t tPeak = anyTime(fNoiseRng);
        // height of the noise maximum after an up-crossing: P(A > a) = exp(-(a^2 - u^2) / (2 sigma^2))
        const Double_t U = 1. - flat(fNoiseRng); // (0, 1]
        const Double_t amplitude = std::sqrt(u * u - 2. * sigma * sigma * std::log(U));
        // near its maximum the noise is a parabola with curvature A (2 pi R0)^2
        const Double_t tCross = tPeak - std::sqrt(2. * (amplitude - u) / amplitude) / twoPiR0;
        Hit hit;
        hit.crossingNs = tCross;
        hit.peakNs = tPeak;
        hit.amplitudeADC = amplitude;
        hit.noise = kTRUE;
        hit.readyNs = ReadyTime(tPeak, 2. * tPeak - tCross); // falling edge: symmetric around the maximum
        fNoiseHits[pad].push_back(hit);
    }
    return nNoise;
}

// ============================================================================
// Signal and noise hits of one pad -> digis, with ONE dead time per channel: ordered by
// the rising edge; a hit whose rising edge is inside the dead time of the previous accepted hit
// (rising edge ... ReadyTime) is lost. With the neighbour logic the accepted hits are kept in
// fSelfHits and the digis are made in AddNeighborDigis (a forced neighbour readout makes the
// channel dead as well).
// ============================================================================
void R3BGTPCVMMDigitizer::WriteHits(Int_t pad, std::vector<Hit>& hits)
{
    std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) { return a.crossingNs < b.crossingNs; });
    Double_t readyAt = -std::numeric_limits<Double_t>::infinity();
    for (const Hit& hit : hits)
    {
        if (hit.crossingNs < readyAt)
            continue;
        if (fNeighborRows || fNeighborColumns)
            fSelfHits.emplace_back(pad, hit);
        else
            CreateDigi(pad, hit.amplitudeADC, hit.peakNs, hit.crossingNs, kFALSE, hit.noise);
        readyAt = hit.readyNs;
    }
}

void R3BGTPCVMMDigitizer::CreateDigi(Int_t pad,
                                     Double_t amplitudeADC,
                                     Double_t peakNs,
                                     Double_t crossingNs,
                                     Bool_t neighbor,
                                     Bool_t noise)
{
    const Bool_t saturated = amplitudeADC >= kADCMax;
    const UShort_t adc = static_cast<UShort_t>(saturated ? kADCMax : std::max(0., std::round(amplitudeADC)));
    const Int_t index = fDigis->GetEntriesFast();

    // MC truth: share of every MC track in the signal at the peak; the rest of the amplitude
    // is electronics noise. A self-triggered digi is a noise hit when the noise is the larger part.
    auto* match = new ((*fDigiMatches)[index]) R3BGTPCMatch();
    if (!noise)
        FillMatch(pad, peakNs, *match);
    const Double_t noisePart = std::max(0., amplitudeADC - match->GetMCWeight());
    if (noisePart > 0.)
        match->AddNoise(noisePart);
    if (!neighbor && match->GetMCWeight() < noisePart)
        noise = kTRUE;

    ++fNDigis;
    fNSaturated += saturated;
    fNNeighborDigis += neighbor;
    fNNoiseDigis += noise;
    // the chip measures ONE time: threshold crossing or peak (neighbour digis: the trigger's time)
    const Double_t timeNs = (fTimeAtThreshold && crossingNs >= 0.) ? crossingNs : peakNs;
    new ((*fDigis)[index]) R3BGTPCVMMDigi(pad, adc, timeNs, saturated, neighbor, noise);
}

// ============================================================================
// Neighbour logic
// ============================================================================

// Every channel that triggered (signal or noise) forces the readout of its neighbours. All hits are
// processed in the order of their rising edges, with the dead time of every channel (VMM3 spec.,
// App. A): a channel is busy from its rising edge until ReadyTime of its hit, and a channel forced to
// read out a neighbour signal is busy from the trigger's rising edge until the trigger's peak +
// digitization + reset (its peak detector holds the neighbour's maximum at that time). A forced
// readout of a busy channel does not happen; a self-triggered hit whose rising edge falls into a
// forced readout of its channel is lost (it does not force its own neighbours either). (A hit that its own channel lost
// only because of such a lost hit is not recovered: rare, both would be within one dead time.) The neighbour channel's
// peak detector holds the maximum of its own waveform (signal + noise) within +- t_p / 2 of the trigger's peak (the
// ATLAS MM digitization searches the peak of a neighbour strip with the threshold lowered). A pad with charge has its
// waveform already; a pad without charge gets a noise-only waveform for that window.
void R3BGTPCVMMDigitizer::AddNeighborDigis()
{
    std::stable_sort(fSelfHits.begin(),
                     fSelfHits.end(),
                     [](const std::pair<Int_t, Hit>& a, const std::pair<Int_t, Hit>& b)
                     { return a.second.crossingNs < b.second.crossingNs; });
    std::unordered_map<Int_t, std::vector<std::pair<Double_t, Double_t>>> busy; // pad -> [from, until) of its readouts
    auto isBusy = [&](Int_t pad, Double_t t)
    {
        const auto it = busy.find(pad);
        if (it == busy.end())
            return false;
        return std::any_of(it->second.begin(),
                           it->second.end(),
                           [&](const std::pair<Double_t, Double_t>& w) { return t >= w.first && t < w.second; });
    };
    std::vector<Double_t> noiseOnly;
    for (const auto& [pad, hit] : fSelfHits)
    {
        if (isBusy(pad, hit.crossingNs)) // forced readout of this channel in progress: its own edge is lost
            continue;
        CreateDigi(pad, hit.amplitudeADC, hit.peakNs, hit.crossingNs, kFALSE, hit.noise);
        busy[pad].emplace_back(hit.crossingNs, hit.readyNs);

        const Int_t row = pad % fNRows, col = pad / fNRows;
        std::vector<Int_t> neighbors;
        if (fNeighborRows)
            for (const Int_t r : { row - 1, row + 1 })
                if (r >= 0 && r < fNRows)
                    neighbors.push_back(col * fNRows + r);
        if (fNeighborColumns)
            for (const Int_t c : { col - 1, col + 1 })
                if (c >= 0 && c < fNColumns)
                    neighbors.push_back(c * fNRows + row);

        const Double_t tPeak = hit.peakNs;
        for (const Int_t nb : neighbors)
        {
            if (isBusy(nb, hit.crossingNs)) // the neighbour channel is dead (own hit or another forced readout)
                continue;
            const Long64_t kA = static_cast<Long64_t>(std::floor((tPeak - 0.5 * fPeakingTimeNs) / fStepNs));
            const Long64_t kB = static_cast<Long64_t>(std::ceil((tPeak + 0.5 * fPeakingTimeNs) / fStepNs));
            Double_t value = -std::numeric_limits<Double_t>::infinity();
            const auto itWave = fWaveforms.find(nb);
            if (itWave != fWaveforms.end())
            {
                const Waveform& wf = itWave->second;
                for (Long64_t k = std::max(kA, wf.firstStep); k <= kB; ++k)
                    if (k - wf.firstStep < static_cast<Long64_t>(wf.values.size()))
                        value = std::max(value, wf.values[static_cast<std::size_t>(k - wf.firstStep)]);
            }
            if (!std::isfinite(value)) // no waveform there: noise only
            {
                NoiseSeries(static_cast<std::size_t>(kB - kA + 1), noiseOnly);
                for (const Double_t x : noiseOnly)
                    value = std::max(value, x);
            }
            // time of the neighbour digi = the time the trigger channel measured (MC truth at the peak)
            CreateDigi(nb, std::max(0., value), tPeak, fTimeAtThreshold ? hit.crossingNs : -1., kTRUE, kFALSE);
            busy[nb].emplace_back(hit.crossingNs, tPeak + fDeadTimeNs);
        }
    }
}

// ============================================================================
// DLC: shaper input of every pad = sum of the inputs of its MC tracks
// ============================================================================
void R3BGTPCVMMDigitizer::MergeTrackInputs()
{
    std::vector<Int_t> pads;
    pads.reserve(fTrackInputs.size());
    for (const auto& entry : fTrackInputs)
        pads.push_back(entry.first);
    std::sort(pads.begin(), pads.end()); // fixed order of the additions
    std::vector<Int_t> ids;
    for (const Int_t pad : pads)
    {
        PadInput& total = fPads[pad];
        auto& tracks = fTrackInputs[pad];
        ids.clear();
        for (const auto& entry : tracks)
            ids.push_back(entry.first);
        std::sort(ids.begin(), ids.end());
        for (const Int_t id : ids)
        {
            const PadInput& in = tracks[id];
            for (std::size_t k = 0; k < in.steps.size(); ++k)
            {
                const GridStep& g = in.steps[k];
                if (!g.hasInput)
                    continue;
                GridStep& t = total.At(in.firstStep + static_cast<Long64_t>(k));
                t.realState += g.realState;
                t.complexState += g.complexState;
                t.currentChange += g.currentChange;
            }
        }
    }
}

// ============================================================================
// MC truth: share of every MC track in the noiseless pulse height at the peak time,
// Q_i h(t_peak - t_i) [ADC] (linear shaper)
// ============================================================================
Double_t R3BGTPCVMMDigitizer::ResponseAt(Double_t dtNs) const
{
    if (dtNs <= 0.)
        return 0.;
    return fK1 * std::exp(-fRealRate * dtNs) + 2. * std::real(fK2 * std::exp(fComplexRate * dtNs));
}

void R3BGTPCVMMDigitizer::FillMatch(Int_t pad, Double_t peakTimeNs, R3BGTPCMatch& match) const
{
    if (fDLCEnabled)
    {
        // noiseless signal of every MC track on this pad, interpolated at the peak exactly as the
        // peak detector output (ChannelHits): parabola through the grid points j - 1, j, j + 1
        const auto itPad = fTrackInputs.find(pad);
        if (itPad == fTrackInputs.end())
            return;
        const Long64_t j = std::llround(peakTimeNs / fStepNs);
        const Double_t dx = peakTimeNs / fStepNs - static_cast<Double_t>(j);
        std::vector<Double_t> v;
        for (const auto& [mcTrack, input] : itPad->second)
        {
            SignalOnGrid(input, j - 1, j + 1, v);
            const Double_t share = v[1] - 0.25 * (v[0] - v[2]) * dx;
            if (share > 0.)
                match.AddLink(fCurrentFile, fCurrentEntry, mcTrack, share);
        }
        return;
    }
    const auto it = fPadElectrons.find(pad);
    if (it == fPadElectrons.end())
        return;
    for (const PadElectron& e : it->second)
    {
        const Double_t share = e.chargeADC * ResponseAt(peakTimeNs - e.timeNs);
        if (share > 0.)
            match.AddLink(fCurrentFile, fCurrentEntry, e.mcTrack, share);
    }
}

// ============================================================================
// Event loop
// ============================================================================
void R3BGTPCVMMDigitizer::Exec(Option_t*)
{
    fDigis->Clear("C");
    fDigiMatches->Delete(); // matches hold std::vectors: destroy them, do not reuse
    fPads.clear();
    fPadElectrons.clear();
    fTrackInputs.clear();
    fDLCDeposits.clear();
    fWaveforms.clear();
    fNoiseHits.clear();
    fSelfHits.clear();
    ++fNEvents;

    // MC event of this entry, for the MC links: input file id and entry in that file
    // (FairFileSource / FairMixedSource fill the event header; without one: file 0, event count)
    FairRun* run = FairRun::Instance();
    FairEventHeader* header = run ? run->GetEventHeader() : nullptr;
    fCurrentFile = header ? header->GetInputFileId() : 0;
    fCurrentEntry = header ? header->GetMCEntryNumber() : static_cast<Int_t>(fNEvents - 1);
    fEventTimeNs = header ? header->GetEventTime() : 0.; // 0 unless the source sets event times

    if (!fGeometryRead && !ReadPadGeometry())
        return;
    const Int_t nPads = GetNumberOfPads();

    // ---- 1. charge of every drift electron on its pad
    for (Int_t ip = 0; ip < fProjPoints->GetEntriesFast(); ++ip)
    {
        const auto* point = dynamic_cast<const R3BGTPCProjPoint*>(fProjPoints->At(ip));
        if (!point)
            continue;
        const Int_t pad = point->GetVirtualPadID();
        if (pad < 0 || pad >= nPads)
        {
            LOG(error) << "R3BGTPCVMMDigitizer: pad id " << pad << " outside map";
            continue;
        }
        const auto& arrivalTimes = point->GetArrivalTimesNs();
        const auto& mcTracks = point->GetElectronTrackIds();
        if (fDLCEnabled)
        {
            // charge spreading on the resistive layer from the landing point of every electron
            if (!point->HasElectronPositions())
            {
                LOG(fatal) << "R3BGTPCVMMDigitizer: the DLC needs the landing point of every electron in "
                              "GTPCProjPoint (re-run the current R3BTPCProjector)";
                return;
            }
            const auto& xs = point->GetElectronXPlaneCm();
            const auto& zs = point->GetElectronZPlaneCm();
            for (std::size_t i = 0; i < arrivalTimes.size(); ++i)
            {
                ++fNElectrons;
                CollectDLCElectron(xs[i], zs[i], fEventTimeNs + arrivalTimes[i], AvalancheADC(), mcTracks[i]);
            }
            continue;
        }
        for (std::size_t i = 0; i < arrivalTimes.size(); ++i)
        {
            ++fNElectrons;
            const Double_t charge = AvalancheADC();
            if (i < mcTracks.size())
                fPadElectrons[pad].push_back({ fEventTimeNs + arrivalTimes[i], charge, mcTracks[i] });
            else
                ++fNElectronsNoTrack;
            AddCharge(pad, fEventTimeNs + arrivalTimes[i], charge); // absolute time
        }
    }

    if (fDLCEnabled)
    {
        SpreadDLCDeposits(); // charge spreading of all deposits of the event
        MergeTrackInputs();
    }

    // ---- 2. noise-only hits on all pads
    if (fNoiseEnabled)
        fNNoiseGenerated +=
            GenerateNoise(fEventTimeNs + fPeakingTimeNs, fEventTimeNs + fPeakingTimeNs + fReadoutWindowNs);

    // ---- 3. pads in a fixed order (reproducible random numbers): signal + noise hits -> digis
    std::vector<Int_t> padIds;
    padIds.reserve(fPads.size() + fNoiseHits.size());
    for (const auto& entry : fPads)
        padIds.push_back(entry.first);
    for (const auto& entry : fNoiseHits)
        if (fPads.find(entry.first) == fPads.end())
            padIds.push_back(entry.first);
    std::sort(padIds.begin(), padIds.end());

    std::vector<Hit> hits;
    for (const Int_t pad : padIds)
    {
        hits.clear();
        const auto itPad = fPads.find(pad);
        if (itPad != fPads.end())
            DigitizePad(pad, itPad->second, hits);
        // noise-only hits (Rice) on this pad, except inside its simulated waveform, which
        // already contains the noise of that time
        const auto itNoise = fNoiseHits.find(pad);
        const auto itWave = fWaveforms.find(pad);
        if (itNoise != fNoiseHits.end())
            for (const Hit& noiseHit : itNoise->second)
            {
                if (itWave != fWaveforms.end())
                {
                    const Double_t t0 = static_cast<Double_t>(itWave->second.firstStep) * fStepNs;
                    const Double_t t1 = t0 + static_cast<Double_t>(itWave->second.values.size()) * fStepNs;
                    if (noiseHit.crossingNs >= t0 && noiseHit.crossingNs <= t1)
                        continue;
                }
                hits.push_back(noiseHit);
            }
        WriteHits(pad, hits);
    }

    // ---- 4. neighbour logic
    if (fNeighborRows || fNeighborColumns)
        AddNeighborDigis();
}

void R3BGTPCVMMDigitizer::Finish()
{
    LOG(info) << "R3BGTPCVMMDigitizer: events " << fNEvents << ", drift electrons " << fNElectrons << ", digis "
              << fNDigis << " (neighbour-triggered " << fNNeighborDigis << ", noise-only " << fNNoiseDigis
              << "), saturated " << fNSaturated;
    if (fNoiseEnabled)
        LOG(info) << "R3BGTPCVMMDigitizer: noise hits generated " << fNNoiseGenerated << ", kept " << fNNoiseDigis
                  << " (rest lost in the dead time), expected per event "
                  << fNoiseRatePerNs * GetNumberOfPads() * fReadoutWindowNs;
    if (fNElectronsNoTrack > 0)
        LOG(warn) << "R3BGTPCVMMDigitizer: " << fNElectronsNoTrack
                  << " drift electrons without an MC track id (older projector file): not in GTPCVMMDigiMatch";

    // run settings, so that the reconstruction and the analysis use exactly these values
    FairSink* sink = FairRootManager::Instance() ? FairRootManager::Instance()->GetSink() : nullptr;
    if (!sink)
        return;
    const std::pair<const char*, Double_t> settings[] = {
        { "VMM3a_peaking_time_ns", fPeakingTimeNs },
        { "VMM3a_dead_time_ns", fDeadTimeNs }, // after the peak (dead from the rising edge)
        { "VMM3a_reset_time_ns", fResetTimeNs },
        { "VMM3a_gain_mV_per_fC", fVMMGainMVPerFC },
        { "VMM3a_ADC_per_mV", fADCCountsPerMV },
        { "VMM3a_detector_gain", fDetectorGain },
        { "VMM3a_polya_theta", fUsePolya ? fPolyaTheta : -1. },
        { "VMM3a_ADC_per_drift_electron", GetADCPerDriftElectron() }, // mean ADC of one drift electron
        { "VMM3a_noise_sigma_ADC", fNoiseSigmaADC },
        { "VMM3a_threshold_ADC", fThresholdADC },
        { "VMM3a_noise_on", fNoiseEnabled ? 1. : 0. },
        { "VMM3a_noise_Kfilter", fNoiseKUsed },
        { "VMM3a_noise_R0_MHz", fNoiseR0PerNs * 1.e3 },
        { "VMM3a_time_jitter_k", fPeakJitterNs / fPeakingTimeNs }, // sigma_t = k t_p sigma_noise / A
        { "VMM3a_noise_rate_Hz", fNoiseRatePerNs * 1.e9 },
        { "VMM3a_readout_window_ns", fReadoutWindowNs },
        { "VMM3a_ADC_per_fC", fVMMGainMVPerFC * fADCCountsPerMV }, // for ana_gtpc_vmm.C (charge in fC)
        { "VMM3a_ENC_e", fENCElectrons },
        { "VMM3a_threshold_e", fThresholdADC / fADCPerElectron },
        { "VMM3a_time_at_threshold", fTimeAtThreshold ? 1. : 0. },
        { "VMM3a_neighbor_rows", fNeighborRows ? 1. : 0. },
        { "VMM3a_neighbor_columns", fNeighborColumns ? 1. : 0. },
        { "VMM3a_DLC_on", fDLCEnabled ? 1. : 0. },
        { "VMM3a_DLC_RC_ns_per_mm2", fDLCEnabled ? fDLCRCnsPerMm2 : 0. },
        { "VMM3a_DLC_sigma0_mm", fDLCEnabled ? fDLCSigma0Mm : 0. },
        { "VMM3a_DLC_ohm_per_square", fDLCEnabled ? fDLCOhmPerSquare : 0. },
        { "VMM3a_DLC_insulator_um", fDLCEnabled ? fDLCInsulatorUm : 0. },
        { "VMM3a_DLC_epsR", fDLCEnabled ? fDLCEpsR : 0. },
    };
    for (const auto& [name, value] : settings)
    {
        TParameter<Double_t> parameter(name, value);
        sink->WriteObject(&parameter, name);
    }
}

ClassImp(R3BGTPCVMMDigitizer)
