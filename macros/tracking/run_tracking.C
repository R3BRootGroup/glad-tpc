void run_tracking(TString fileName = "output_reco.root")
{

    TStopwatch timer;
    timer.Start();

    // Input file: simulation
    TString inFile;
    // Input file: parameters
    TString parFile;
    // Output file
    TString outFile;

    TString GTPCTrackParamsFile;
    TString workDir = gSystem->Getenv("VMCWORKDIR");

    inFile = workDir + "/glad-tpc/macros/reco/" + fileName;
    outFile = workDir + "/glad-tpc/macros/tracking/output_tracking.root";
    GTPCTrackParamsFile = workDir + "/glad-tpc/params/Hit_FileSetup.par";

    // -----   Create analysis run   ----------------------------------------
    FairRunAna* fRun = new FairRunAna();
    fRun->SetSource(new FairFileSource(inFile));
    fRun->SetOutputFile(outFile.Data());

    // -----   Runtime database   ---------------------------------------------
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIn = new FairParRootFileIo(kTRUE);
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    // parIn->open(parFile.Data());
    parIo1->open(GTPCTrackParamsFile, "in");
    // rtdb->setFirstInput(parIn);
    rtdb->setSecondInput(parIo1);
    rtdb->print();

    R3BGTPCHit2Track* hit2cal = new R3BGTPCHit2Track();

    fRun->AddTask(hit2cal);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
