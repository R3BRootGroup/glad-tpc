void run_reconstruction(TString fileName = "proj.root")
{
    TStopwatch timer;
    timer.Start();

    // Input file: simulation
    TString inFile;
    // Input file: parameters
    TString parFile;
    // Output file
    TString outFile;

    TString GTPCHitParamsFile;
    TString workDir = gSystem->Getenv("VMCWORKDIR");

    inFile = workDir + "/glad-tpc/macros/proj/Prototype/" + fileName;
    outFile = workDir + "/glad-tpc/macros/reco/output_reco.root";
    GTPCHitParamsFile = workDir + "/glad-tpc/params/Hit_FileSetup.par";

    // -----   Create analysis run   ----------------------------------------
    FairRunAna* fRun = new FairRunAna();
    fRun->SetSource(new FairFileSource(inFile));
    fRun->SetOutputFile(outFile.Data());

    // -----   Runtime database   ---------------------------------------------
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIn = new FairParRootFileIo(kTRUE);
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    // parIn->open(parFile.Data());
    parIo1->open(GTPCHitParamsFile, "in");
    // rtdb->setFirstInput(parIn);
    rtdb->setSecondInput(parIo1);
    rtdb->print();

    R3BGTPCCal2Hit* cal2hit = new R3BGTPCCal2Hit();

    fRun->AddTask(cal2hit);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
