{
  gROOT->ProcessLine(".L readVPadPlane.C");
  reader("proj.root");
  guiForPads(0);
}
