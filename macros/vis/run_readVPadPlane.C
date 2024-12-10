{
  gROOT->ProcessLine(".L readVPadPlane.C");
  reader("../proj/Prototype/proj.root"); // change the folder according to the
                                         // detector you want to use
  guiForPads(0);
}
