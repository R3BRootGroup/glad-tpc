{
    gROOT->ProcessLine(".L simHYDRA.C");
    simHYDRA(10000, "Prototype", "good_evt");
}
// simHYDRA(nevt,"Detector","generator")
// Detector: "Prototype","FullBeamIn"
// Generator: "good_evt", "bkg_evt", "box" TODO signal+bkg
// nevt:bkg 20455 only if if bkg_evt is chosen should be set this number of
// events, this corresponds to 1 sec of carbon 12 beam (10^5pps) that impinges
// on a c12 target
