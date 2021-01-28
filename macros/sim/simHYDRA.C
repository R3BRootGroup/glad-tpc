void simHYDRA(Int_t nEvents = 0, TString GEOTAG = "Prototype")//n. events and GEOTAG to be definied in run_simHYDRA.c
{
    TString transport = "TGeant4";

    TString outFile;
    TString parFile;

    Bool_t magnet = kTRUE;
    Float_t fieldScale = 1;
    TString generator = "ascii";
    TString inputFile;

 if (GEOTAG.CompareTo("Prototype") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inputFile =
            "/home/simone/Programs/R3B/R3BRoot/glad-tpc/gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        outFile="./Prototype/sim.root";
        parFile="./Prototype/par.root";

    }
        
    if (GEOTAG.CompareTo("Fullv1") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inputFile =
            "/home/simone/Programs/R3B/R3BRoot/glad-tpc/gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        parFile="./Fullv1/par.root";
        outFile="./Fullv1/sim.root";

    }
    if (GEOTAG.CompareTo("Fullv2") == 0)
    {
        cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;
        inputFile =
            "/home/simone/Programs/R3B/R3BRoot/glad-tpc/gtpcgen/ASCII/input" + GEOTAG + "_3LH.dat";
        parFile="./Fullv2/par.root";
        outFile="./Fullv2/sim.root";

    }
    //else exit(0);

    Bool_t storeTrajectories = kTRUE;
    Int_t randomSeed = 335566; // 0 for time-dependent random numbers

    // ------------------------------------------------------------------------
    // Stable part ------------------------------------------------------------

    TString dir = getenv("VMCWORKDIR");
    char str[1000];
    sprintf(str, "GEOMPATH=%s/glad-tpc/geometry", dir.Data());
    putenv(str);

    // ----    Debug option   -------------------------------------------------
    gDebug = 0;

    // -----   Timer   --------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName(transport);            // Transport engine
    run->SetOutputFile(outFile.Data()); // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media_tpc.geo"); // Materials

    // -----   Create R3B geometry --------------------------------------------
    // R3B Cave definition
    FairModule* cave = new R3BCave("CAVE");
    cave->SetGeometryFileName("r3b_cave.geo");
    run->AddModule(cave);

    // To skip the detector comment out the line with: run->AddModule(...

    // GLAD
    run->AddModule(new R3BGladMagnet("glad_v17_flange.geo.root")); // GLAD should not be moved or rotated

    // --- GLAD-TPC detectors ---
    if (GEOTAG.CompareTo("Prototype") == 0)	run->AddModule(new R3BGTPC("HYDRA_Prototype.geo.root")); 
    if (GEOTAG.CompareTo("Fullv1") == 0)		run->AddModule(new R3BGTPC("HYDRA_Fullv1.geo.root"));    
    if (GEOTAG.CompareTo("Fullv2") == 0)		run->AddModule(new R3BGTPC("HYDRA_Fullv2.geo.root"));            

    // -----   Create R3B  magnetic field ----------------------------------------
    // NB: <D.B>
    // If the Global Position of the Magnet is changed
    // the Field Map has to be transformed accordingly
    R3BGladFieldMap* magField = new R3BGladFieldMap("R3BGladMap");
    magField->SetScale(fieldScale);

    if (magnet == kTRUE)
    {
        run->SetField(magField);
    }
    else
    {
        run->SetField(NULL);
    }

    // -----   Create PrimaryGenerator   --------------------------------------
    // 1 - Create the Main API class for the Generator
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();

    if (generator.CompareTo("ascii") == 0)
    {
        R3BAsciiGenerator* gen = new R3BAsciiGenerator((inputFile).Data());
        primGen->AddGenerator(gen);
    }

    run->SetGenerator(primGen);

    run->SetStoreTraj(storeTrajectories);

    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");

    // -----   Initialize simulation run   ------------------------------------
    run->Init();
    TVirtualMC::GetMC()->SetRandom(new TRandom3(randomSeed));

    // ------  Increase nb of step for CALO
    Int_t nSteps = -15000;
    TVirtualMC::GetMC()->SetMaxNStep(nSteps);

    // -----   Runtime database   ---------------------------------------------
    R3BFieldPar* fieldPar = (R3BFieldPar*)rtdb->getContainer("R3BFieldPar");
    if (NULL != magField)
    {
        fieldPar->SetParameters(magField);
        fieldPar->setChanged();
    }
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();

    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
    {
        run->Run(nEvents);
    }

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;

    // Snap a picture of the geometry
    // If this crashes, set "OpenGL.SavePicturesViaFBO: no" in your .rootrc

    /*
        gStyle->SetCanvasPreferGL(kTRUE);
        gGeoManager->GetTopVolume()->Draw("ogl");
        TGLViewer* v = (TGLViewer*)gPad->GetViewer3D();
        v->SetStyle(TGLRnrCtx::kOutline);
        v->RequestDraw();
        v->SavePicture("run_sim-side.png");
        v->SetPerspectiveCamera(TGLViewer::kCameraPerspXOZ, 25., 0, 0, -90. * TMath::DegToRad(), 0. *
       TMath::DegToRad()); v->SavePicture("run_sim-top.png");*/
}
