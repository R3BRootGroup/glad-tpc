void run_proj(TString GEOTAG = "FullBeamIn", TString version= "v1")
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
        outFile = "./Prototype/proj.root";
    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/FullBeamOut/sim.root";
        parFile = "../sim/FullBeamOut/par.root"; 
        outFile = "./FullBeamOut/proj.root";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        if (version.CompareTo("v1")==0) 
        {
				    inFile = "../sim/FullBeamIn/v1/sim.root";
				    parFile = "../sim/FullBeamIn/v1/par.root";
				    outFile = "./FullBeamIn/v1/proj.root";
        }
    		if (version.CompareTo("v2")==0)
    		{
				    inFile = "../sim/FullBeamIn/v2/sim.root";
				    parFile = "../sim/FullBeamIn/v2/par.root";
				    outFile = "./FullBeamIn/v2/proj.root";
        }
    		if (version.CompareTo("v3")==0) 
    		{
				    inFile = "../sim/FullBeamIn/v3/sim.root";
				    parFile = "../sim/FullBeamIn/v3/par.root";
				    outFile = "./FullBeamIn/v3/proj.root";
        }
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

    R3BGTPCProjector* pro = new R3BGTPCProjector();
    R3BGTPCSetup *setup=new R3BGTPCSetup();
    pro->SetDriftParameters(setup->GetEIonization(), setup->GetDriftVelocity(), setup->GetLongDiff(), setup->GetTransDiff(), setup->GetFanoFactor());
    pro->SetSizeOfVirtualPad(setup->GetPadSize()); // 1 means pads of 1cm^2, 10 means pads of 1mm^2...

    fRun->AddTask(pro);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    if(timer.RealTime()<60.)cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
    else cout << "Real time: " << timer.RealTime()/60. << "min, CPU time: " << timer.CpuTime()/60 << "min" << endl;
}
