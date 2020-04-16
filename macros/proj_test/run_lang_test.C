void run_lang_test() {
  TStopwatch timer;
  timer.Start();

  // Input file: simulation
  TString inFile = "../sim/sim_test_100.root";
  // Input file: parameters
  TString parFile = "../sim/par_test_100.root";
  // Output file
  TString outFile = "borrame.root";

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

  R3BGTPCLangevinTest* lantest = new R3BGTPCLangevinTest();

  fRun->AddTask(lantest);

  fRun->Init();
  fRun->Run(0, 0);
  delete fRun;

  timer.Stop();

  cout << "Macro finished succesfully!" << endl;
  cout << "Output file writen: " << outFile << endl;
  cout << "Parameter file writen: " << parFile << endl;
  cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
