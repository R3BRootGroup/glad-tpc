void run_proj(TString GEOTAG = "Prototype")
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
    if (GEOTAG.CompareTo("Fullv1") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/Fullv1/sim.root";
        parFile = "../sim/Fullv1/par.root"; 
        outFile = "./Fullv1/proj.root";
    }
    if (GEOTAG.CompareTo("Fullv2") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/Fullv2/sim.root";
        parFile = "../sim/Fullv2/par.root";
        outFile = "./Fullv2/proj.root";
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
    //ionization[GeV], drift velocity[cm/ns], longDiff[cm^(-1/2)], transvDiff[cm^(-1/2)], Fanofactor
    pro->SetDriftParameters(15.e-9, 0.005, 0.0000001, 0.000001, 2);
    pro->SetSizeOfVirtualPad(10); // 1 means pads of 1cm^2, 10 means pads of 1mm^2...

    fRun->AddTask(pro);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file writen: " << outFile << endl;
    cout << "Parameter file writen: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
