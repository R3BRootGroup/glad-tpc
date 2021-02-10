{
    gROOT->ProcessLine(".L simHYDRA.C");
    simHYDRA(10,"FullBeamIn","v1","good_evt");
}
//simHYDRA(nevt,"Detector","version","generator")
//Detector: "Prototype", "FullBeamOut", "FullBeamIn"
//version: For FullBeamIn we have 3 different versions
//Generator: "good_evt", "bkg_evt", "box"
//nevt:bkg 20455 <-if bkg_evt is chosen should be set this number of events, this corresponds to carbon 12 beam (10^6 apps) that impinges on a c12 target
