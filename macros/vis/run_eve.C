void run_eve(TString InputDataFile = "output_tracking.root",
             TString OutputDataFile = "output.reco_display.root",
             TString unpackDir = "/glad-tpc/macros/tracking/") {
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger->SetLogToScreen(kTRUE);
  fLogger->SetLogVerbosityLevel("MEDIUM");
  TString dir = getenv("VMCWORKDIR");
  TString geoFile = "HYDRA_Prototype.geoMan.root";

  TString InputDataPath = dir + unpackDir + InputDataFile;
  TString OutputDataPath = dir + unpackDir + OutputDataFile;
  TString GeoDataPath = dir + "/glad-tpc/geometry/" + geoFile;

  // FairRunAna* fRun = new FairRunAna();
  // fRun->SetInputFile(InputDataPath);
  // fRun->SetOutputFile(OutputDataPath);
  // fRun->SetGeomFile(GeoDataPath);

  FairRunAna *fRun = new FairRunAna();
  FairRootFileSink *sink = new FairRootFileSink(OutputDataPath);
  FairFileSource *source = new FairFileSource(InputDataPath);
  fRun->SetSource(source);
  fRun->SetSink(sink);
  fRun->SetGeomFile(GeoDataPath);

  FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo *parIo1 = new FairParRootFileIo();
  // parIo1->open("param.dummy.root");
  rtdb->setFirstInput(parIo1);

  FairRootManager *ioman = FairRootManager::Instance();

  R3BGTPCEventManager *eveMan = new R3BGTPCEventManager();
  R3BGTPCEventDrawTask *eve = new R3BGTPCEventDrawTask();

  eveMan->AddTask(eve);
  eveMan->Init();
}
