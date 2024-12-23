void run_reconstruction(TString fileName = "lang.root")
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
    // TString GTPCHitParamsFile;
    TString GTPCGeoParamsFile;
    TString workDir = gSystem->Getenv("VMCWORKDIR");

    cout << "\033[1;31m Warning\033[0m: The detector is: Prototype" << endl;
    inFile = workDir + "/glad-tpc/macros/proj/Prototype/" + fileName;
    outFile = workDir + "/glad-tpc/macros/reco/output_reco.root";
    // GTPCHitParamsFile = workDir + "/glad-tpc/params/Hit_FileSetup.par";
    parFile = "../sim/Prototype/par.root";
    GTPCGeoParamsFile = workDir + "/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par";

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

    R3BGTPCCal2Hit* cal2hit = new R3BGTPCCal2Hit();
    //(David)
    if (fileName == "proj.root")
    {
        cal2hit->SetRecoFlag(kFALSE);
    }

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
