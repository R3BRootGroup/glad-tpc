{
    gROOT->ProcessLine(".L Padplane_readout.C");
    reader("proj.root");//change the folder according to the detector you want to use
}
