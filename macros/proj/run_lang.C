void run_lang(TString GEOTAG = "Prototype")
{
    TStopwatch timer;
    timer.Start();

    // Input file: simulation
    TString inFile;
    // Input file: parameters
    TString parFile;
    // Output file
    TString outFile;

    // Input and outup file according to the GEOTAG
    if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/Prototype/sim.root"; 
        parFile = "../sim/Prototype/par.root"; 
        outFile = "./Prototype/lang.root";
    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/FullBeamOut/sim.root"; 
        parFile = "../sim/FullBeamOut/par.root";
        outFile = "./FullBeamOut/lang.root";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/FullBeamIn/sim.root"; 
        parFile = "../sim/FullBeamIn/par.root"; 
        outFile = "./FullBeamIn/lang.root";
    }

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

    R3BGTPCLangevin* lan = new R3BGTPCLangevin();
    R3BGTPCSetup *setup=new R3BGTPCSetup();
    lan->SetDriftParameters(setup->GetEIonization(), setup->GetDriftVelocity(), setup->GetLongDiff(), setup->GetTransDiff(), setup->GetFanoFactor());
    lan->SetSizeOfVirtualPad(setup->GetPadSize()); // 1 means pads of 1cm^2, 10 means pads of 1mm^2, ...

    fRun->AddTask(lan);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
