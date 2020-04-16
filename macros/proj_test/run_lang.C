void run_lang() {
  TStopwatch timer;
  timer.Start();

  // Input file: simulation
  TString inFile = "../sim/sim2.root";
  // Input file: parameters
  TString parFile = "../sim/par2.root";
  // Output file
  TString outFile = "lang2.root";

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

  R3BGTPCLangevin* lan = new R3BGTPCLangevin();

  fRun->AddTask(lan);

  fRun->Init();
  fRun->Run(0, 0);
  delete fRun;

  timer.Stop();

  cout << "Macro finished succesfully!" << endl;
  cout << "Output file writen: " << outFile << endl;
  cout << "Parameter file writen: " << parFile << endl;
  cout << "Real time: " << timer.RealTime() << "s, CPU time: " << timer.CpuTime() << "s" << endl;
}
