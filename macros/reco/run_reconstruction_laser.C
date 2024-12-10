void run_reconstruction_laser(TString fileName = "laser_gen_gladField.root") {
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
  outFile = workDir + "/glad-tpc/macros/reco/output_reco_laser.root";
  // GTPCHitParamsFile = workDir + "/glad-tpc/params/Hit_FileSetup.par";
  parFile = "../sim/Prototype/par.root";
  GTPCGeoParamsFile =
      workDir + "/glad-tpc/params/HYDRAprototype_FileSetup_v2_02082022.par";

  // -----   Create analysis run   ----------------------------------------
  FairRunAna *fRun = new FairRunAna();
  fRun->SetSource(new FairFileSource(inFile));
  fRun->SetOutputFile(outFile.Data());

  // ----- Magnetic filed -------------------------------------------------

  FairField *magField = NULL;

  Bool_t constB = kFALSE;

  if (constB) {
    magField = new R3BFieldConst();
    double B_y = 100.; //[kG]
    ((R3BFieldConst *)magField)->SetField(0., B_y, 0.);
    ((R3BFieldConst *)magField)
        ->SetFieldRegion(-200.0, // x_min
                         200.0,  // x_max
                         -100.0, // y_min
                         100.0,  // y_max
                         -150.0, // z_min
                         450.0); // z_max

  } else {
    magField = new R3BGladFieldMap("R3BGladMap");
    ((R3BGladFieldMap *)(magField))->SetScale(0.4185489744);
    ((R3BGladFieldMap *)(magField))->Init();
  }

  fRun->SetField(magField);
  fRun->SetSource(new FairFileSource(inFile));
  fRun->SetOutputFile(outFile.Data());

  // -----   Runtime database   ---------------------------------------------
  FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo *parIn = new FairParRootFileIo(kTRUE);
  FairParAsciiFileIo *parIo1 = new FairParAsciiFileIo(); // Ascii file
  parIn->open(parFile.Data());
  parIo1->open(GTPCGeoParamsFile, "in");
  rtdb->setFirstInput(parIn);
  rtdb->setSecondInput(parIo1);
  rtdb->print();

  R3BGTPCCal2Hit *cal2hit = new R3BGTPCCal2Hit();
  //(David)
  if (fileName == "proj.root") {
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
  cout << "Real time: " << timer.RealTime()
       << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
