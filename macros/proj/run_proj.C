void run_proj() {
  TStopwatch timer;
  timer.Start();

  // Input file: simulation
  TString inFile = "../sim/sim.root";
  // Input file: parameters
  TString parFile = "../sim/par.root";
  // Output file
  TString outFile = "proj.root";

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
  pro->SetDriftParameters(15.e-9, 0.005, 0.0000001, 0.000001, 2);
  pro->SetSizeOfVirtualPad(1); //1 means pads of 1cm^2, 10 means pads of 1mm^2, ...

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
