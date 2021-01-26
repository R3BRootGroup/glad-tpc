void run_lang_test()
{
    TStopwatch timer;
    timer.Start();

    // Input file: simulation
    TString inFile = "../sim/sim.root";
    // Input file: parameters
    TString parFile = "../sim/par.root";
    // Output file
    TString outFile = "lang_test.root";

    // -----   Create analysis run   ----------------------------------------
    FairRunAna* fRun = new FairRunAna();
    fRun->SetSource(new FairFileSource(inFile));
    fRun->SetOutputFile(outFile.Data());

    // -----   Runtime database   ---------------------------------------------
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIn = new FairParRootFileIo(kTRUE);
    parIn->open(parFile.Data());
    rtdb->setFirstInput(parIn);
    rtdb->print();

    R3BGTPCLangevinTest* lantest = new R3BGTPCLangevinTest();
    lantest->SetDriftParameters(15.e-9, 0.0048, 0.00000216, 0.00000216, 2);
    lantest->SetSizeOfVirtualPad(100); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...
    lantest->SetNumberOfGeneratedElectrons(10);

    fRun->AddTask(lantest);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
