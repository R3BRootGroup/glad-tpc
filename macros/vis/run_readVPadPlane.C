{
  gROOT->ProcessLine(".L readVPadPlane.C");
  reader("../proj/proj.root");
  guiForPads(0);
}
