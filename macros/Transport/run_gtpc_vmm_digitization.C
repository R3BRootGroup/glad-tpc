// ============================================================================
// run_gtpc_vmm_baseline.C
//
// HYDRA/GLAD-TPC VMM3a digitization (R3BGTPCVMMDigitizer). All charges in ADC counts.
//
// Input : proj.root written by R3BTPCProjector with SetProjPointsAsOutput()
//         (GTPCProjPoint + GTPCMap; pad geometry comes from there).
// Output: GTPCVMMDigi (pad, ADC, ONE time: threshold crossing or peak, flags) + GTPCVMMDigiMatch
//
// Threshold: THL in mV above the baseline (as set on the VMM3a), or, if thresholdMV <= 0,
// thresholdSigma x noise. The digitizer prints it in ADC, sigma and mV at Init().
//
// Noise (enableNoise), electronics noise sigma = ENC (HYDRA with HV on: ~5000 e, arXiv:2606.22607):
//   - a Gaussian noise PROCESS in time (series noise shaped by the VMM3a shaper, rms = ENC),
//     generated for every pad with charge and its neighbours and added to the signal; the
//     threshold, peak detector and neighbour logic act on signal + noise as on the chip
//   - pads without charge: noise-only hits from the Rice formula, R = R0 exp(-u^2 / (2 sigma^2)),
//     R0 = K_filter / t_p; noiseKFilter <= 0: K_filter from the VMM3a shaper (0.519).
//     They are generated in the readout window of the event = drift length / drift velocity,
//     taken from the geometry (geoFile) and GTPCGasPar (parAsciiFile): nothing to set.
//
// Dead time (VMM3 specifications v1.0, App. A, Figs. 21 and 23, 10-bit ADC data path):
//     rising edge |-- rise to the peak --| peak |-- 220 ns digitization --|-- 30 ns reset --| ready
// The dead time of a channel starts at the rising edge (threshold crossing) and ends deadTimeNs = 250 ns
// after the peak (the 10-bit ADC converts the peak value = the charge in 220 ns, then 30 ns reset). If the
// signal is still above threshold longer, the reset (resetTimeNs = 30 ns) follows the falling edge:
//     channel ready again = max(peak + deadTimeNs, falling edge + resetTimeNs)
// A new hit needs a new rising edge: a signal that starts in the dead time is lost, and so is a signal
// starting before that lost one has gone below threshold; overlapping signals (pile-up) give one hit.
// With t_p = 200 ns the channel is dead ~400 ns from the rising edge (Table 8: t_p + 220 + 30 ns).
// With the neighbour logic, a channel forced to read out for its neighbour is dead from the trigger's
// rising edge until the trigger's peak + deadTimeNs.
//
// Timing (VMM3a register): the chip measures ONE time per hit, at the threshold crossing
// (timeAtThreshold = kTRUE) or at the peak; the reconstruction reads which one from the file.
//
// Neighbour logic (VMM3a): a channel that crosses the threshold also reads out its neighbours.
// neighborRows: row +- 1 (next pad along X), neighborColumns: column +- 1 (next pad along Z).
// If any is on, the automatic output name gets "_NBrows" / "_NBcols".
//
// Resistive anode (DLC) of the Micromegas (enableDLC): the avalanche charge spreads on the DLC,
// sigma^2(t) = sigma0^2 + 2 t / (R C) (Dixit & Rankin), and every pad gets its own pulse from the
// charge above it as a function of time. Values of the HYDRA-TPC (L. Ji, PhD thesis 2025):
//   R      = 1.35 MOhm/square   DLC sheet resistance (measured 1.2 - 1.5 MOhm/square)
//   d1     = 128 um, eps = 1    Micromegas gap (micromesh -> DLC), gas
//   d2     = 100 um, eps = 2.5  epoxy glue (DLC -> pads)
//   C      = capacitance per area of the DLC sheet to ground. The DLC is a conductor between two
//            grounded electrodes (pads below, micromesh above; the mesh is at AC ground through its
//            HV filter), so the two gaps are capacitors IN PARALLEL (Dixit & Rankin, T2K ERAM):
//              C = eps0 (e2 / d2 + e1 / d1) = 8.854e-15 (2.5 / 0.100 + 1 / 0.128) = 2.90e-13 F/mm^2
//   RC     = 1.35e6 ohm * 2.90e-13 F/mm^2 = 3.92e-7 s/mm^2 = 392 ns/mm^2
//            (thesis eq. 2.53 puts the two gaps in SERIES, eps0 e1 e2 / (e1 d2 + e2 d1) = 5.27e-14
//             F/mm^2, RC = 71 ns/mm^2: that is the capacitance mesh -> pads, not the one the DLC sheet
//             charges; it spreads the charge 2.3 x wider (sigma ~ 2 mm = 1 pad pitch within t_p))
//   sigma0 = 0.10 mm            cloud width at the DLC from GEM + Micromegas (thesis: ~100 um)
// Units: RC in ns/mm^2 and sigma0 in mm here (as usually quoted); the digitizer converts them once
// to cm (all lengths in the digitizer and the reconstruction are in cm, times in ns).
// Needs the landing point of every electron in GTPCProjPoint (R3BTPCProjector, ProjPoint mode).
// The output name gets "_DLC<RC>".
// ============================================================================

void run_gtpc_vmm_digitization(
    const TString inputFile = "../../gtpc_muon_150GeV_G4.proj.root",
    const TString parFile = "../../gtpc_muon_150GeV_G4.par.root",
    TString outputFile = "test_digi_without_DLC.root", // empty: name from the settings below
    Int_t nEvents = -1,

    // Detector
    Double_t detectorGain = 10000,
    Bool_t enablePolya = kTRUE,
    Double_t polyaTheta = 1.0,

    // VMM3a settings
    Double_t vmmGainMVPerFC = 3.0,  // 0.5, 1, 3, 4.5, 6, 9, 12, 16
    Double_t peakingTimeNs = 200.0, // 25, 50, 100, 200
    Double_t deadTimeNs = 250.0,    // after the peak: 220 ns digitization + 30 ns reset
    Double_t resetTimeNs = 30.0,    // reset part; after the falling edge if ToT is longer
    Double_t adcCountsPerMV = 1.0,  // ADC calibration
    Double_t encElectrons = 5000.0, // ENC [e rms]; HYDRA with detector HV on
    Double_t thresholdMV = 15.0,    // THL [mV above baseline]; <= 0: use thresholdSigma
    Double_t thresholdSigma = 0.0,  // used only if thresholdMV <= 0

    Bool_t timeAtThreshold = kTRUE, // VMM3a timing: threshold crossing (kTRUE) or peak (kFALSE)

    // Electronics noise
    Bool_t enableNoise = kTRUE,
    Double_t noiseKFilter = 0.52, // K_filter, R0 = K / t_p; <= 0: from the VMM3a shaper (0.519)

    // VMM3a neighbour logic
    Bool_t neighborRows = kFALSE,
    Bool_t neighborColumns = kFALSE,

    // setup: readout window = drift length (geometry) / drift velocity (GTPCGasPar)
    const TString geoFile = "../../gtpc_muon_150GeV_G4.geo.root",
    const TString parAsciiFile =
        "/home/ag-obertelli/r3broot/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par", // same .par as the
                                                                                               // projector

    // Resistive anode (DLC) of the Micromegas, HYDRA-TPC values (L. Ji thesis 2025)
    // This is to be improved in a to do task
    Bool_t enableDLC = kFALSE,
    Double_t dlcOhmPerSquare = 1.35e6, // DLC sheet resistance [ohm/square]
    Double_t dlcMeshGapUm = 128.,      // micromesh -> DLC gap [um], gas (eps = 1)
    Double_t dlcGlueUm = 100.,         // epoxy glue DLC -> pads [um]
    Double_t dlcGlueEpsR = 2.5,        // relative permittivity of the glue
    Double_t dlcSigma0Mm = 0.10)       // cloud width at the DLC [mm]
{
    // DLC: capacitance per area of gas gap + glue (thesis eq. 2.53) and RC [ns/mm^2]
    const Double_t eps0FPerMm = 8.8541878128e-15;                         // F/mm
    const Double_t d1Mm = 1.e-3 * dlcMeshGapUm, d2Mm = 1.e-3 * dlcGlueUm; // um -> mm
    // DLC -> pads (glue) and DLC -> micromesh (gas gap) in parallel (see the header) [F/mm^2]
    const Double_t dlcCFPerMm2 = eps0FPerMm * (dlcGlueEpsR / d2Mm + 1. / d1Mm);
    const Double_t dlcRCnsPerMm2 = dlcOhmPerSquare * dlcCFPerMm2 * 1.e9; // ohm F/mm^2 = s/mm^2 -> ns/mm^2

    if (outputFile == "")
    {
        outputFile = thresholdMV > 0. ? Form("gtpc_muon_150GeV_G4.vmm_THL%.0fmV_ENC%.0fe_%s.root",
                                             thresholdMV,
                                             encElectrons,
                                             enableNoise ? "noise" : "nonoise")
                                      : Form("gtpc_muon_150GeV_G4.vmm_THL%.1fsigma_ENC%.0fe_%s.root",
                                             thresholdSigma,
                                             encElectrons,
                                             enableNoise ? "noise" : "nonoise");
        if (neighborRows || neighborColumns)
            outputFile.Insert(outputFile.Length() - 5,
                              TString(neighborRows ? "_NBrows" : "") + (neighborColumns ? "_NBcols" : ""));
        if (enableDLC)
            outputFile.Insert(outputFile.Length() - 5, Form("_DLC%.0f", dlcRCnsPerMm2));
    }

    auto* run = new FairRunAna();
    run->SetSource(new FairFileSource(inputFile));
    run->SetSink(new FairRootFileSink(outputFile));
    run->SetGeomFile(geoFile);

    auto* parInput = new FairParRootFileIo(kTRUE);
    if (!parInput->open(parFile.Data(), "READ"))
    {
        std::cerr << "ERROR: cannot open parameter file " << parFile << "\n";
        return;
    }
    run->GetRuntimeDb()->setFirstInput(parInput);
    auto* parAscii = new FairParAsciiFileIo();
    if (!parAscii->open(parAsciiFile.Data(), "in"))
    {
        std::cerr << "ERROR: cannot open ASCII parameter file " << parAsciiFile << "\n";
        return;
    }
    run->GetRuntimeDb()->setSecondInput(parAscii);

    auto* vmm = new R3BGTPCVMMDigitizer();

    // Detector
    vmm->SetDetectorGain(detectorGain);
    if (enablePolya)
        vmm->SetPolyaTheta(polyaTheta);

    // VMM3a
    vmm->SetVMMGain(vmmGainMVPerFC);
    vmm->SetPeakingTime(peakingTimeNs);
    vmm->SetDeadTime(deadTimeNs);   // [ns] after the peak (the channel is dead from the rising edge)
    vmm->SetResetTime(resetTimeNs); // [ns]
    vmm->SetADCCountsPerMV(adcCountsPerMV);
    vmm->SetENC(encElectrons);
    if (thresholdMV > 0.)
        vmm->SetThresholdMV(thresholdMV);
    else
        vmm->SetThresholdSigma(thresholdSigma);

    // Electronics noise
    vmm->SetTimeAtThreshold(timeAtThreshold);
    vmm->EnableNoise(enableNoise);
    if (noiseKFilter > 0.)
        vmm->SetNoiseKFilter(noiseKFilter);

    // VMM3a neighbour logic
    vmm->SetNeighborLogic(neighborRows, neighborColumns);

    // Resistive anode (DLC)
    if (enableDLC)
    {
        vmm->SetDLCRC(dlcRCnsPerMm2);         // [ns/mm^2]
        vmm->SetDLCInitialSigma(dlcSigma0Mm); // [mm]
        std::cout << "DLC: R = " << dlcOhmPerSquare << " ohm/sq, C = " << dlcCFPerMm2
                  << " F/mm^2, RC = " << dlcRCnsPerMm2 << " ns/mm^2, sigma0 = " << dlcSigma0Mm << " mm\n";
    }

    vmm->SetRandomSeed(12345);
    vmm->SetPersistence(kTRUE);
    run->AddTask(vmm);

    run->Init();

    TStopwatch timer;
    timer.Start();
    if (nEvents < 0)
        run->Run();
    else
        run->Run(0, nEvents);
    timer.Stop();

    std::cout << "\nVMM3a digitization finished: " << outputFile << "\n"
              << "  noise sigma       : " << vmm->GetNoiseSigmaADC() << " ADC\n"
              << "  threshold         : " << vmm->GetThresholdADC() << " ADC = " << vmm->GetThresholdSigma()
              << " sigma\n"
              << "  1 drift electron  : " << vmm->GetADCPerDriftElectron() << " ADC (mean)\n"
              << "  noise             : " << (enableNoise ? "on" : "off") << "\n"
              << "  timing            : " << (timeAtThreshold ? "threshold crossing" : "peak") << "\n"
              << "  dead time         : rising edge -> peak + " << deadTimeNs << " ns (reset " << resetTimeNs
              << " ns after the falling edge if later)\n"
              << "  neighbour logic   : rows " << (neighborRows ? "on" : "off") << ", columns "
              << (neighborColumns ? "on" : "off") << "\n"
              << "  DLC               : "
              << (enableDLC ? Form("RC = %.1f ns/mm^2, sigma0 = %.2f mm", vmm->GetDLCRCnsPerMm2(), dlcSigma0Mm) : "off")
              << "\n"
              << "  pads              : " << vmm->GetNumberOfPads() << "\n"
              << "  real " << timer.RealTime() << " s, cpu " << timer.CpuTime() << " s\n";
}