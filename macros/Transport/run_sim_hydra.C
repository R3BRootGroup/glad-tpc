void run_sim_hydra(TString input = "",
                   Int_t nEvents = 1000,
                   TString output = "gtpc_muon_150GeV",
                   TString transportEngine = "TGeant4", // TGeant4 | TGeant3
                   TString field = "none",              // none | const | glad
                   Int_t randomSeed = 0,                // 0 => time-dependent
                   Bool_t overwrite = kTRUE,
                   Bool_t storeTrajectories = kFALSE,
                   UInt_t runId = 0,
                   Int_t pdg = 13, // 13 = mu-, -13 = mu+
                   Double_t momentum = 150.0,
                   Double_t startX = 0.0,
                   Double_t startY = 0.0,
                   Double_t startZ = 0.0,
                   Bool_t IsBoxGenerator = kTRUE, // kFALSE => ASCII file `input`
                   Int_t multiplicity = 1,
                   TString geoTag = "",
                   // Beam spot around (startX, startY) [cm]; all 0 = pencil beam.
                   // Gaussian (realistic beam profile):
                   Double_t beamWidthX = 6.0, // across the pad columns
                   Double_t beamWidthY = 10.0 // over the drift length
)
{

    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");

    // -----   Environment   --------------------------------------------------
    TString myName = "Event Transport"; // this macro's name for screen output

    // ----- Paths & checks --------------------------------------------------
    const char* workDir = gSystem->Getenv("VMCWORKDIR");
    if (!workDir)
    {
        std::cerr << "VMCWORKDIR not set. Source R3BRoot config.sh first." << std::endl;
        return;
    }
    const TString geoDir = TString(workDir) + "/glad-tpc/geometry";
    gSystem->Setenv("GEOMPATH", geoDir); // FairRoot looks for .geo/.root here first

    if (transportEngine != "TGeant4" && transportEngine != "TGeant3")
    {
        std::cerr << "transportEngine must be TGeant4 or TGeant3" << std::endl;
        return;
    }

    TString tag = output + (transportEngine == "TGeant4" ? "_G4" : "_G3");
    TString outFile = tag + ".tra.root";
    TString geoFile = tag + ".geo.root";
    TString parFile = tag + ".par.root";

    if (!overwrite && !gSystem->AccessPathName(outFile))
    {
        std::cerr << outFile << " exists and overwrite = kFALSE." << std::endl;
        return;
    }
    gSystem->mkdir(gSystem->GetDirName(outFile), kTRUE);

    // ----- Run -------------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    auto* run = new FairRunSim();
    run->SetName(transportEngine);
    // run->SetOutputFile(outFile.Data());
    run->SetSink(new FairRootFileSink(outFile)); // Output file
    // run->SetGenerateRunInfo(kTRUE);  // Create FairRunInfo file

    // -----   Create media   -------------------------------------------------
    std::cout << std::endl;
    std::cout << "-I- " << myName << ": Setting media file" << std::endl;
    run->SetMaterials("media_tpc.geo"); // Materials
    // ------------------------------------------------------------------------

    // -----   Create R3B geometry --------------------------------------------
    // R3B Cave definition
    FairModule* cave = new R3BCave("CAVE");
    cave->SetGeometryFileName("r3b_cave.geo");
    run->AddModule(cave);

    // run->AddModule(new R3BGladMagnet("glad_s455_v2023.1.geo.root")); // GLAD should not be moved or rotated

    // Prototype: shifted to x = 8.6 cm, z = 271 cm
    // run->AddModule(new R3BGTPC("HYDRA_Prototype.geo.root", TGeoTranslation(8.6, 0., 271.)));
    // run->AddModule(
    //     new R3BGTPC("/home/ag-obertelli/r3broot/glad-tpc/geometry/HYDRA_Prototype_CERNtest_noTarget.geo.root"));

    auto* gtpc = new R3BGTPC("geometry/HYDRA_Prototype_CERNtest.geo.root");

    run->AddModule(gtpc);

    // -----   Create PrimaryGenerator   --------------------------------------
    std::cout << std::endl;
    std::cout << "-I- " << myName << ": Registering event generators" << std::endl;

    auto* primGenerator = new FairPrimaryGenerator(); // FairPrimaryGenerator

    Bool_t smearVertexXY = kTRUE;
    Bool_t smearVertexZ = kTRUE;

    primGenerator->SetBeam(0., 0., beamWidthX, beamWidthY);
    primGenerator->SmearGausVertexXY(smearVertexXY);
    primGenerator->SmearVertexZ(smearVertexZ);
    // primGen->SetBeamAngle(0, 0, 1.7e-3, 1.7e-3); //Gaussian sigma of beam [rad]

    // ----- Primary generator ----------------------------------------------
    if (IsBoxGenerator)
    {

        FairBoxGenerator* boxGen = new FairBoxGenerator(pdg, multiplicity);
        boxGen->SetPRange(momentum, momentum); // pZ momentum GeV/c

        boxGen->SetThetaRange(0.0, 0.0);
        boxGen->SetPhiRange(0.0, 0.0);

        boxGen->SetXYZ(startX, startY, startZ);
        primGenerator->AddGenerator(boxGen);
    }
    run->SetGenerator(primGenerator);

    // ----- Field -----------------------------------------------------------
    FairField* magField = nullptr; // nullptr => no field

    if (field == "const")
    {
        auto* f = new R3BFieldConst();
        f->SetField(0., 20., 0.); // By = 20 kG
        f->SetFieldRegion(-200., 200., -100., 100., -150., 450.);
        magField = f;
    }
    else if (field == "glad")
    {
        auto* f = new R3BGladFieldMap("R3BGladMap");
        f->SetScale(-1.);
        magField = f;
    }
    run->SetField(magField); // nullptr is fine

    // ... generator, run->Init(), etc. ...

    run->Init();

    // ----- Seed the transport engine (after Init) -------------------------
    TVirtualMC* mc = TVirtualMC::GetMC();
    if (!mc)
    {
        std::cerr << "No MC engine after Init() - check transport name/gconfig" << std::endl;
        return;
    }
    mc->SetRandom(new TRandom3(randomSeed)); // ROOT-side random numbers

    // ----- Parameters ------------------------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    if (magField) // true only if a field was created
    {
        auto* fieldPar = static_cast<R3BFieldPar*>(rtdb->getContainer("R3BFieldPar"));
        fieldPar->SetParameters(magField);
        fieldPar->setChanged();
    }
    auto* parOut = new FairParRootFileIo(kTRUE);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();

    // run->SetRandomSeed(randomSeed); // 0 => time-dependent
    run->SetStoreTraj(storeTrajectories);
    // if (runId != 0) run->SetRunId(runId);

    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
    {
        run->Run(nEvents);
    }

    std::cout << "Field object: " << (run->GetField() ? run->GetField()->GetName() : "none") << std::endl;

    run->CreateGeometryFile(geoFile.Data());

    // // ----- Summary ---------------------------------------------------------
    std::cout << "\n=== GLAD-TPC transport ===\n";
    if (IsBoxGenerator)
    {
        std::cout << " Particle   : "
                  << " (PDG " << pdg << ") x" << multiplicity << "\n"
                  << " Momentum   : " << momentum << " GeV/c\n"
                  << " Beam centre: (" << startX << ", " << startY << ", " << startZ << ") cm\n"
                  << " Beam spot  : Gaussian sigma (" << beamWidthX << ", " << beamWidthY << ") cm\n";
        //<< ", uniform half-width (" << beamHalfX << ", " << beamHalfY << ") cm\n";
    }

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished successfully." << endl;
    cout << "Output file is " << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
}