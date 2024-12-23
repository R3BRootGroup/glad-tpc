void eventDisplay()
{
    FairRunAna* fRun = new FairRunAna();

    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    FairParRootFileIo* parIo1 = new FairParRootFileIo();
    parIo1->open("par.root");
    rtdb->setFirstInput(parIo1);
    rtdb->print();

    // fRun->SetInputFile("sim.root");
    // fRun->SetOutputFile("test.root");

    FairRootFileSink* sink = new FairRootFileSink("test.root");
    FairFileSource* source = new FairFileSource("sim.root");
    fRun->SetSource(source);
    fRun->SetSink(sink);
    // fRun->SetGeomFile(GeoDataPath);

    FairEventManager* fMan = new FairEventManager();
    FairMCTracksDraw* Track = new FairMCTracksDraw();
    FairMCPointDraw* GTPCPoints = new FairMCPointDraw("GTPCPoint", kOrange, kFullSquare);

    fMan->AddTask(Track);
    fMan->AddTask(GTPCPoints);

    fMan->Init();
}
