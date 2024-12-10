/*
Author: Simone Velardita
Date:		27/04/2021
Macro for running the simulation.
HOW TO USE:
    $	root -l
    $	.L simHYDRA.C
    $	simHYDRA(nevt,"Detector","generator")
ALTERNATIVE:
    use the macro run_simHYDRA.C: root -l run_simHYDRA.C
*/
void simHYDRA(Int_t nEvents = 1000, TString GEOTAG = "Prototype",
              TString generator = "good_evt") {
  Bool_t storeTrajectories = kTRUE; //  To store particle trajectories
  Bool_t magnet = kTRUE;            //	Switch on/off the B field
  Bool_t constBfield = kTRUE;       //	Constant magnetic field
  Bool_t printGLAD =
      kFALSE; //	print the inner glad vessel and the HYDRA detector
  Float_t fieldScale = -1.;

  TString transport = "TGeant4";
  cout << "The generator used is:\033[1;32m" << generator << endl;
  TString inputFile;
  TString outFile = "./" + GEOTAG + "/sim.root";
  TString parFile = "./" + GEOTAG + "/par.root";

  cout << "\033[1;31m Warning\033[0m: The detector is: " << GEOTAG << endl;

  if (generator.CompareTo("good_evt") == 0)
    inputFile = "../../gtpcgen/ASCII/input" + GEOTAG + "_He3pi_paper.dat";
  if (generator.CompareTo("bkg_evt") == 0)
    inputFile = "../../gtpcgen/ASCII/input" + GEOTAG + "_bkg.dat";
  cout << "File generator:" << inputFile << endl;
  Int_t randomSeed = 335566; // 0 for time-dependent random numbers

  // ------------------------------------------------------------------------

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
  FairRunSim *run = new FairRunSim();
  run->SetName(transport);            // Transport engine
  run->SetOutputFile(outFile.Data()); // Output file
  FairRuntimeDb *rtdb = run->GetRuntimeDb();

  // -----   Create media   -------------------------------------------------
  run->SetMaterials("media_tpc.geo"); // Materials

  // -----   Create R3B geometry --------------------------------------------
  // R3B Cave definition
  FairModule *cave = new R3BCave("CAVE");
  cave->SetGeometryFileName("r3b_cave.geo");
  run->AddModule(cave);

  // To skip the detector comment out the line with: run->AddModule(...

  // GLAD
  run->AddModule(new R3BGladMagnet(
      "glad_s455_v2023.1.geo.root")); // GLAD should not be moved or rotated

  // --- GLAD-TPC detectors
  if (GEOTAG.CompareTo("Prototype") == 0) {
    run->AddModule(new R3BTarget("C12 target", "passive/Target.geo.root",
                                 {-2.7, 0., 227.}, {"", 90., 4, 90.}));
    run->AddModule(new R3BGTPC("HYDRA_Prototype.geo.root", {8.6, 0., 271}));
  } else if (GEOTAG.CompareTo("FullBeamIn") == 0) {
    run->AddModule(
        new R3BTarget("C12target", "passive/Target.geo.root", {0., 0., 170}));
    run->AddModule(new R3BGTPC("HYDRA_FullBeamIn.geo.root")); // position TBD
  }

  // -----   Create R3B  magnetic field ----------------------------------------
  // NB: <D.B>
  // If the Global Position of the Magnet is changed
  // the Field Map has to be transformed accordingly
  R3BGladFieldMap *magField = new R3BGladFieldMap("R3BGladMap");
  magField->SetScale(fieldScale);

  // Constant Magnetic field
  R3BFieldConst *constField = new R3BFieldConst();
  double B_y = 20.; //[kG]
  constField->SetField(0., B_y, 0.);
  constField->SetFieldRegion(-200.0, // x_min
                             200.0,  // x_max
                             -100.0, // y_min
                             100.0,  // y_max
                             -150.0, // z_min
                             450.0); // z_max

  if (magnet) {
    if (constBfield)
      run->SetField(constField);
    else
      run->SetField(magField);
  } else
    run->SetField(NULL);

  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  // Box to be used for debugging
  if (generator.CompareTo("box") == 0) {
    Int_t pdgId = -211;    // pi-
    Double32_t theta1 = 0; // polar angle distribution
    Double32_t theta2 = 1.;
    Double32_t momentum = 0.8; //[GeV]
    FairBoxGenerator *boxGen = new FairBoxGenerator(pdgId, 1);
    boxGen->SetThetaRange(theta1, theta2);
    boxGen->SetPRange(momentum, momentum * 1.0);
    boxGen->SetPhiRange(0, 360);
    boxGen->SetXYZ(-2.7, 0.0, 237); // target position+10cm decay length
    primGen->AddGenerator(boxGen);
  }

  if (generator.CompareTo("bkg_evt") == 0 ||
      generator.CompareTo("good_evt") == 0) {
    R3BAsciiGenerator *gen = new R3BAsciiGenerator((inputFile).Data());
    primGen->AddGenerator(gen);
  }
  run->SetGenerator(primGen);
  run->SetStoreTraj(storeTrajectories);

  FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
  FairLogger::GetLogger()->SetLogScreenLevel(
      "INFO"); // nolog for no output, INFO to show the n. of hits

  // -----   Initialize simulation run   ------------------------------------
  run->Init();
  TVirtualMC::GetMC()->SetRandom(new TRandom3(randomSeed));

  // ------  Increase nb of step for CALO
  Int_t nSteps = -15000;
  TVirtualMC::GetMC()->SetMaxNStep(nSteps);

  // -----   Runtime database   ---------------------------------------------
  R3BFieldPar *fieldPar = (R3BFieldPar *)rtdb->getContainer("R3BFieldPar");
  if (NULL != magField) {
    if (constBfield)
      fieldPar->SetParameters(constField);
    else
      fieldPar->SetParameters(magField);

    fieldPar->setChanged();
  }
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo *parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open(parFile.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();

  // -----   Start run   ----------------------------------------------------
  if (nEvents > 0) {
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
  cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl
       << endl;

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  if (printGLAD) {
    // Snap a picture of the geometry
    // If this crashes, set "OpenGL.SavePicturesViaFBO: no" in your .rootrc or
    // just comment
    gStyle->SetCanvasPreferGL(kTRUE);
    gGeoManager->GetTopVolume()->Draw("ogl");
    TGLViewer *v = (TGLViewer *)gPad->GetViewer3D();
    v->SetStyle(TGLRnrCtx::kOutline);
    v->GetClipSet()->SetClipType(TGLClip::EType(1));
    v->RequestDraw();
    v->SetCurrentCamera(TGLViewer::kCameraOrthoXnOZ);
    v->SavePicture("GLAD-top.png");
  }
}
