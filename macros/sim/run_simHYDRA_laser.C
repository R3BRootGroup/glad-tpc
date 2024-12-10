{
  gROOT->ProcessLine(".L simHYDRA.C");
  simHYDRA(1, "Prototype", "good_evt");
}
// The purpose of this macro is to simply generate the event structure
// that will later be filled with the run_laser_gen.C macro. (This is
// the reason why only one event is simulated).

// simHYDRA(nevt,"Detector","generator")
// Detector: "Prototype","FullBeamIn"
// Generator: "good_evt", "bkg_evt", "box" TODO signal+bkg
// nevt:bkg 20455 only if if bkg_evt is chosen should be set this number of
// events, this corresponds to 1 sec of carbon 12 beam (10^5pps) that impinges
// on a c12 target
