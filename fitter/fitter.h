#include "AbsFitterInfo.h"
#include "AbsKalmanFitter.h"
#include "ConstField.h"
#include "DAF.h"
#include "EventDisplay.h"
#include "Exception.h"
#include "FieldManager.h"
#include "FitStatus.h"
#include "HelixTrackModel.h"
#include "KalmanFitStatus.h"
#include "KalmanFitterInfo.h"
#include "KalmanFitterRefTrack.h"
#include "MaterialEffects.h"
#include "MeasuredStateOnPlane.h"
#include "MeasurementFactory.h"
#include "MeasurementOnPlane.h"
#include "MeasurementProducer.h"
#include "PlanarMeasurement.h"
#include "TGeoMaterialInterface.h"
#include "TrackPoint.h"

#include <ios>
#include <iostream>
#include <istream>
#include <limits>
#include <map>
#include <vector>

#include "TCanvas.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TGeoManager.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreePlayer.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TApplication.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TMath.h"

#include "Fit/Fitter.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"
#include "TArrayD.h"
#include "TMatrixD.h"
#include "TRotation.h"
#include "TVectorD.h"

#include "Math/GenVector/AxisAngle.h"
#include "Math/GenVector/EulerAngles.h"
#include "Math/GenVector/Quaternion.h"
#include "Math/GenVector/Rotation3D.h"
#include "Math/GenVector/RotationX.h"
#include "Math/GenVector/RotationY.h"
#include "Math/GenVector/RotationZ.h"
#include "Math/GenVector/RotationZYX.h"

#include "R3BGTPCHitData.h"

class HelixTrackModel
{

  public:
    // Constructors/Destructors ---------
    HelixTrackModel(const TVector3& pos, const TVector3& mom, double charge);

    TVector3 getPos(double tracklength) const;
    void getPosMom(double tracklength, TVector3& pos, TVector3& mom) const;
    void getPosDir(double tracklength, TVector3& pos, TVector3& dir) const
    {
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
