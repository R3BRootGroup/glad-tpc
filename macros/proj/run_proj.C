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

    cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
    inFile = "../sim/" + GEOTAG + "/sim.root";
    parFile = "../sim/" + GEOTAG + "/par.root";
    outFile = "./" + GEOTAG + "/proj.root";
    GTPCGeoParamsFile = geoPath + "/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par";

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
    if (!geoPar)
    {
        cout << "No R3BGTPCGeoPar can be loaded from the rtdb";
        return;
    }
    R3BGTPCGasPar* gasPar = (R3BGTPCGasPar*)rtdb->getContainer("GTPCGasPar");
    if (!gasPar)
    {
        cout << "No R3BGTPCGasPar can be loaded from the rtdb";
        return;
    }

    R3BGTPCProjector* pro = new R3BGTPCProjector();
    pro->SetDriftParameters(gasPar->GetEIonization(),
                            gasPar->GetDriftVelocity(),
                            gasPar->GetLongDiff(),
                            gasPar->GetTransDiff(),
                            gasPar->GetFanoFactor());
    pro->SetProjPointsAsOutput();
    fRun->AddTask(pro);

    fRun->Init();
    fRun->Run(0, 0);
    delete fRun;

    timer.Stop();

    cout << "Macro finished succesfully!" << endl;
    cout << "Output file written: " << outFile << endl;
    cout << "Parameter file written: " << parFile << endl;
    cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
