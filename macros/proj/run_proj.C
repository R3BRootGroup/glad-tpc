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
    TString GTPCGeoParamsFile;
    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/Prototype/sim.root";
        parFile = "../sim/Prototype/par.root";
        outFile = "./Prototype/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup.par";

    }
    if (GEOTAG.CompareTo("FullBeamOut") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inFile = "../sim/FullBeamOut/sim.root";
        parFile = "../sim/FullBeamOut/par.root";
        outFile = "./FullBeamOut/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAFullBeamOut_FileSetup.par";
    }
    if (GEOTAG.CompareTo("FullBeamIn") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;

        inFile = "../sim/FullBeamIn/sim.root";
        parFile = "../sim/FullBeamIn/par.root";
        outFile = "./FullBeamIn/proj.root";
        GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAFullBeamIn_FileSetup.par";
    }
    GTPCGeoParamsFile.ReplaceAll("//", "/");

    // -----   Create analysis run   ----------------------------------------
    FairRunAna* fRun = new FairRunAna();
    fRun->SetSource(new FairFileSource(inFile));
    fRun->SetOutputFile(outFile.Data());

    // -----   Runtime database   ---------------------------------------------
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIn = new FairParRootFileIo(kTRUE);
    FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo(); // Ascii file
    parIn->open(parFile.Data());
    parIo1->open(GTPCGeoParamsFile, "in");
    rtdb->setFirstInput(parIn);
    rtdb->setSecondInput(parIo1);
    rtdb->print();

     R3BGTPCGeoPar* geoPar = (R3BGTPCGeoPar*)rtdb->getContainer("GTPCGeoPar");
    if (!geoPar) {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }
    R3BGTPCGasPar* gasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!gasPar) {
        cout << "No R3BGTPCGasPar can be loaded from the rtdb";
        return;
	}


  
    R3BGTPCProjector* pro = new R3BGTPCProjector();
    pro->SetDriftParameters(gasPar->GetEIonization(),
                            gasPar->GetDriftVelocity(),
                            gasPar->GetLongDiff(),
                            gasPar->GetTransDiff(),
                            gasPar->GetFanoFactor());
    pro->SetSizeOfVirtualPad(geoPar->GetPadSize()); // 1 means pads of 1cm^2, 10 means pads of 1mm^2...
    

    
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
