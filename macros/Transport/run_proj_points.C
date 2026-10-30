
void run_proj_points(TString transportFile = "gtpc_muon_150GeV_G4.tra.root",
                     Int_t nEvents = -1,
                     Int_t readoutSide = +1,
                     Int_t outputMode = 1,
                     Int_t rowsX = 44,
                     Int_t columnsZ = 128,
                     Double_t padSizeXcm = 0.19,
                     Double_t padSizeZcm = 0.19,
                     Double_t gapXcm = 0.01,
                     Double_t gapZcm = 0.01,
                     Bool_t useManualGas = kFALSE)
{
    TString parFile = "gtpc_muon_150GeV_G4.par.root";
    TString outputFile = "gtpc_muon_150GeV_G4.proj.root";

    const char* base = gSystem->Getenv("VMCWORKDIR");
    if (!base)
    {
        std::cerr << "ERROR: VMCWORKDIR not defined\n";
        return;
    }

    TString asciiFile = TString(base) + "/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par";

    if ((readoutSide != 1 && readoutSide != -1) || (outputMode != 0 && outputMode != 1))
    {
        std::cerr << "ERROR: invalid readoutSide/outputMode\n";
        return;
    }
    if (gSystem->AccessPathName(transportFile, kReadPermission) || gSystem->AccessPathName(parFile, kReadPermission) ||
        gSystem->AccessPathName(parFile, kWritePermission) || gSystem->AccessPathName(asciiFile, kReadPermission))
    {
        std::cerr << "ERROR: cannot access transport/parameter files\n";
        return;
    }

    TStopwatch timer;
    timer.Start();
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");

    auto* run = new FairRunAna();
    run->SetSource(new FairFileSource(transportFile));
    run->SetSink(new FairRootFileSink(outputFile));
    auto* rtdb = run->GetRuntimeDb();

    auto* parIn = new FairParRootFileIo(kTRUE);
    if (!parIn->open(parFile.Data(), "READ"))
    {
        std::cerr << "ERROR: cannot read parameter ROOT file\n";
        delete run;
        return;
    }
    rtdb->setFirstInput(parIn);

    auto* parAscii = new FairParAsciiFileIo();
    if (!parAscii->open(asciiFile.Data(), "in"))
    {
        std::cerr << "ERROR: cannot read ASCII parameter file\n";
        delete run;
        return;
    }
    rtdb->setSecondInput(parAscii);

    const Double_t W = 26.0e-9;  // GeV
    const Double_t v = 0.0078;   // cm/ns
    const Double_t DT = 4.0e-6;  // cm^2/ns
    const Double_t DL = 1.74e-6; // cm^2/ns
    const Double_t fano = 0.2;

    auto* projector = new R3BTPCProjector();
    projector->SetReadoutSide(readoutSide);
    projector->SetPadGeometry(rowsX, columnsZ, padSizeXcm, padSizeZcm, gapXcm, gapZcm);
    if (useManualGas)
        projector->SetDriftParameters(W, v, DT, DL, fano);
    if (outputMode == 1)
        projector->SetProjPointsAsOutput();
    else
        projector->SetCalDataAsOutput();
    run->AddTask(projector);

    run->Init();

    // Snapshot the gas values ACTUALLY selected for projection.
    // Never save the pre-Init container: FairRoot can initialize it again.
    auto* gas = dynamic_cast<R3BGTPCGasPar*>(rtdb->getContainer("GTPCGasPar"));
    if (!gas)
    {
        std::cerr << "ERROR: GTPCGasPar is missing\n";
        delete run;
        return;
    }
    R3BGTPCGasPar gasToSave;
    gasToSave.SetEIonization(useManualGas ? W : gas->GetEIonization());
    gasToSave.SetDriftVelocity(useManualGas ? v : gas->GetDriftVelocity());
    gasToSave.SetTransDiff(useManualGas ? DT : gas->GetTransDiff());
    gasToSave.SetLongDiff(useManualGas ? DL : gas->GetLongDiff());
    gasToSave.SetFanoFactor(useManualGas ? fano : gas->GetFanoFactor());

    run->Run(0, nEvents < 0 ? 0 : nEvents);
    delete run; // Close FairRoot's READ handle before modifying the file.

    // Targeted ROOT-object write. Does not invoke FairRuntimeDb::saveOutput(),
    // which previously crashed while serializing FairGeoParSet.
    TFile parUpdate(parFile.Data(), "UPDATE");
    if (parUpdate.IsZombie() || !parUpdate.IsWritable())
    {
        std::cerr << "ERROR: cannot update existing " << parFile << "\n";
        return;
    }
    if (gasToSave.Write("GTPCGasPar", TObject::kOverwrite) <= 0)
    {
        std::cerr << "ERROR: writing GTPCGasPar failed\n";
        return;
    }
    parUpdate.Close();

    // Verify the standalone top-level key required by ana_gtpc_vmm.C.
    TFile verify(parFile.Data(), "READ");
    auto* check = verify.IsZombie() ? nullptr : dynamic_cast<R3BGTPCGasPar*>(verify.Get("GTPCGasPar"));
    if (!check || check->GetDriftVelocity() != gasToSave.GetDriftVelocity() ||
        check->GetFanoFactor() != gasToSave.GetFanoFactor())
    {
        std::cerr << "ERROR: gas parameter verification failed\n";
        return;
    }

    timer.Stop();
    std::cout << "Projection finished: " << outputFile << "\n"
              << "GTPCGasPar key updated in: " << parFile << "\n"
              << "Saved drift velocity: " << check->GetDriftVelocity() << " cm/ns\n"
              << "Saved Fano factor: " << check->GetFanoFactor() << "\n"
              << "Real time: " << timer.RealTime() << " s\n";
}