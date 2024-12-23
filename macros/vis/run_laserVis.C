{
    gROOT->ProcessLine(".L laserVis.C");
    laserVis("../proj/Prototype/laser_gen_gladField.root",
             "gladField",
             "RECREATE"); // change the folder according to the detector you want
                          // to use
}
