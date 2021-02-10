{
    gROOT->ProcessLine(".L Electronics.C");
    reader("../proj/Prototype/proj.root");//Change the folder based on the detector you want to use
}
