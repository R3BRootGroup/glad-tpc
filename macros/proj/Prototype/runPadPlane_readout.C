{
    gROOT->ProcessLine(".L Padplane_readout_cal.C");
    reader("lang.root"); // change the folder according to the detector you want to use
}
