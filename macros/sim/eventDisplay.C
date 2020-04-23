void eventDisplay()
{
  FairRunAna *fRun= new FairRunAna();

  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo*  parIo1 = new FairParRootFileIo();
  parIo1->open("par.root");
  rtdb->setFirstInput(parIo1);
  rtdb->print();

  fRun->SetInputFile("sim.root");
  fRun->SetOutputFile("test.root");

  FairEventManager *fMan= new FairEventManager();
  FairMCTracks *Track =  new FairMCTracks ("Monte-Carlo Tracks");
  FairMCPointDraw *GTPCPoints =   new FairMCPointDraw ("GTPCPoints",kOrange,  kFullSquare);
  R3BCalifaEventDisplay    *CalifaEvtVis    = new R3BCalifaEventDisplay("R3BCalifaEventDisplay");
  R3BCalifaHitEventDisplay *CalifaHitEvtVis = new R3BCalifaHitEventDisplay("R3BCalifaHitEventDisplay");
  CalifaEvtVis->SelectGeometryVersion(10);
  //FairMCPointDraw *LandPoints =   new FairMCPointDraw ("NeulandPoints",kOrange,  kFullSquare);

  fMan->AddTask(Track);

  fMan->AddTask(GTPCPoints);
  fMan->AddTask(CalifaEvtVis);
  fMan->AddTask(CalifaHitEvtVis);
  //fMan->AddTask(LandPoints);

  fMan->Init();
}
