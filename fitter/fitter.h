#include "AbsKalmanFitter.h"
#include "KalmanFitterRefTrack.h"
#include "DAF.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "MaterialEffects.h"
#include "TGeoMaterialInterface.h"
#include "MeasurementFactory.h"
#include "MeasurementProducer.h"
#include "EventDisplay.h"
#include "KalmanFitStatus.h"
#include "FitStatus.h"
#include "AbsFitterInfo.h"
#include "KalmanFitterInfo.h"
#include "MeasuredStateOnPlane.h"
#include "MeasurementOnPlane.h"
#include "TrackPoint.h"
#include "Exception.h"
#include "HelixTrackModel.h"
#include "PlanarMeasurement.h"
#include "TrackPoint.h"

#include <ios>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <vector>

#include "TClonesArray.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreePlayer.h"
#include "TTreeReaderValue.h"
#include "TSystem.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStopwatch.h"
#include "TGeoManager.h"


#include "TGraph.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TMath.h"
#include "TF1.h"
#include "TAxis.h"

#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Fit/Fitter.h"
#include "TRotation.h"
#include "TMatrixD.h"
#include "TArrayD.h"
#include "TVectorD.h"


#include "Math/GenVector/Rotation3D.h"
#include "Math/GenVector/EulerAngles.h"
#include "Math/GenVector/AxisAngle.h"
#include "Math/GenVector/Quaternion.h"
#include "Math/GenVector/RotationX.h"
#include "Math/GenVector/RotationY.h"
#include "Math/GenVector/RotationZ.h"
#include "Math/GenVector/RotationZYX.h"

#include "R3BGTPCHitData.h"

class HelixTrackModel{

 public:

  // Constructors/Destructors ---------
  HelixTrackModel(const TVector3& pos, const TVector3& mom, double charge);

  TVector3 getPos(double tracklength) const;
  void getPosMom(double tracklength, TVector3& pos, TVector3& mom) const;
  void getPosDir(double tracklength, TVector3& pos, TVector3& dir) const {
    getPosMom(tracklength, pos, dir);
    dir.SetMag(1);
  }


 private:

  double sgn_;
  double mom_;
  double R_; // radius
  TVector3 center_;
  double alpha0_;
  double theta_;


 public:
  

};
