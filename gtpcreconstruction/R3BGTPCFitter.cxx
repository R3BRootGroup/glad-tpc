#include "R3BGTPCFitter.h"
#include "GFBField.h"
#include "R3BGTPCHitClusterData.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCSpacePointMeasurement.h"
#include "R3BGTPCTrackData.h"

// GENFIT
#include <AbsBField.h>
#include <AbsKalmanFitter.h>
#include <AbsMeasurement.h>
#include <AbsTrackRep.h>
#include <ConstField.h>
#include <Exception.h>
#include <FieldManager.h>
#include <FitStatus.h>
#include <KalmanFittedStateOnPlane.h>
#include <KalmanFitter.h>
#include <KalmanFitterRefTrack.h>
#include <MaterialEffects.h>
#include <MeasuredStateOnPlane.h>
#include <MeasurementFactory.h>
#include <MeasurementProducer.h>
#include <Track.h>

#include <Math/Vector3D.h>
#include <Math/Vector3Dfwd.h>
#include <Rtypes.h>

#include "TFile.h"
#include "TRandom3.h"
#include <Math/Point3D.h>
#include <RKTrackRep.h>
#include <TClonesArray.h>
#include <TDatabasePDG.h>
#include <TGeoManager.h>
#include <TGeoMaterial.h>
#include <TGeoMaterialInterface.h>
#include <TGeoMedium.h>
#include <TGeoVolume.h>
#include <TMath.h>
#include <TMatrixDSymfwd.h>
#include <TMatrixDfwd.h>
#include <TMatrixT.h>
#include <TMatrixTSym.h>
#include <TObjArray.h>
#include <TObject.h>
#include <TROOT.h>
#include <TVector3.h>
#include <TrackCand.h>

R3BGTPCFitter::R3BGTPCFitter()
{
    fTPCDetID = 0;
    fPDGCode = 211;

    fKalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
    fKalmanFitter->setMinIterations(5);
    fKalmanFitter->setMaxIterations(20);
    fKalmanFitter->setDeltaPval(1.E-3);

    fGenfitTrackArray = new TClonesArray("genfit::Track");
    fHitClusterArray = new TClonesArray("R3BGTPCHitClusterData");

    fMeasurementProducer =
        new genfit::MeasurementProducer<R3BGTPCHitClusterData, genfit::R3BGTPCSpacepointMeasurement>(fHitClusterArray);
    fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
    fMeasurementFactory->addProducer(fTPCDetID, fMeasurementProducer);

    // genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.0, 20.0, 0.0)); // kGauss
    // new field map
    auto fieldPointer = GFBField::GetInstance("R3BGladMap_Bxyz_X-3to3_Y-1to1_Z-4to13_step10mm", "R", 0, 1.75, 163.4);
    // old field map
    // auto fieldPointer = GFBField::GetInstance("R3BGladMap","A",0,1.75,163.4);
    genfit::FieldManager::getInstance()->init(fieldPointer);

    genfit::MaterialEffects* materialEffects = genfit::MaterialEffects::getInstance();
    materialEffects->init(new genfit::TGeoMaterialInterface());
}

R3BGTPCFitter::~R3BGTPCFitter()
{

    delete fGenfitTrackArray;
    delete fHitClusterArray;
    delete fMeasurementProducer;
    delete fMeasurementFactory;
}

void R3BGTPCFitter::Init()
{
    std::cout << cGREEN << " R3BGTPCFitter::Init() " << cNORMAL << "\n";

    fHitClusterArray->Delete();
    fGenfitTrackArray->Delete();
}

genfit::Track* R3BGTPCFitter::FitTrack(R3BGTPCTrackData* track, Double_t TPC_Pin, Double_t TPC_Theta, Double_t TPC_Phi)
{
    fHitClusterArray->Delete();
    genfit::TrackCand trackCand;

    auto hitClusterArray = track->GetHitClusterArray();

    TVector3 pos_res;
    TVector3 mom_res;
    TMatrixDSym cov_res;

    Double_t resolution[3];

    resolution[0] = 0.04; // 0.2/5 0.2 is  pad size
    resolution[1] = 0.04; // same
    resolution[2] = 0.1;  //

    gRandom = new TRandom3();
    gRandom->SetSeed(0);
    TRandom3 r1;
    r1.SetSeed(0);
    const double momSmear = 0.00;
    const double momMagSmear = 0.0;

    std::cout << cYELLOW << " Track " << track->GetTrackId() << " with " << hitClusterArray->size() << " clusters "
              << cNORMAL << "\n";

    Int_t Nmeas;
    Nmeas = hitClusterArray->size();

    for (auto cluster : *hitClusterArray)
    {
        Int_t idx = fHitClusterArray->GetEntriesFast();
        new ((*fHitClusterArray)[idx]) R3BGTPCHitClusterData(cluster);
        trackCand.addHit(fTPCDetID, idx);
    }

    auto iniCluster = hitClusterArray->front();
    auto endCluster = hitClusterArray->back();

    std::cout << " Initial cluster " << iniCluster.GetX() << "  " << iniCluster.GetY() << "  " << iniCluster.GetZ()
              << "\n";
    // std::cout<<" End cluster "<<endCluster.GetX()<<"  "<<endCluster.GetY()<<"  "<<endCluster.GetZ()<<"\n";

    TVector3 posSeed(iniCluster.GetX(), iniCluster.GetY(), iniCluster.GetZ());
    posSeed.SetMag(posSeed.Mag());

    TMatrixDSym covSeed(6); //
    TMatrixD covMatrix = iniCluster.GetCovMatrix();
    for (Int_t iComp = 0; iComp < 3; iComp++)
    {
        //    covSeed(iComp, iComp) = 0.4; // covMatrix(iComp, iComp);
        covSeed(iComp, iComp) = resolution[iComp] * resolution[iComp];
        // std::cout <<"COVSEED  " << covSeed(iComp, iComp) << "\n";
    }

    for (Int_t iComp = 3; iComp < 6; iComp++)
    {
        // covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);
        covSeed(iComp, iComp) = pow(resolution[iComp - 3] / Nmeas / sqrt(3), 2);
        // covSeed(iComp,iComp) = 0.04;
    }

    // TVector3 momSeed;
    TVector3 momSeed(0.0, 0.0, 1);
    // TVector3 momSeed(0.0,0.0,0.8);
    momSeed.SetMag(r1.Gaus(TPC_Pin, momMagSmear * TPC_Pin));
    momSeed.SetTheta(r1.Gaus(TPC_Theta, momSmear));
    momSeed.SetPhi(r1.Gaus(TPC_Phi, momSmear));

    // momSeed.SetTheta(gRandom->Gaus(TPC_Theta,momSmear));
    // momSeed.SetMag(gRandom->Gaus(TPC_Pin,momMagSmear*TPC_Pin));
    // momSeed.SetTheta(18.96*TMath::Pi()/180.);
    /*std::cout << TPC_Theta << "   " << momSeed.Theta() << std::endl;
    std::cout << TPC_Phi << "   " << momSeed.Phi() << std::endl;
    std::cout << momSmear << "   " << momMagSmear << std::endl;*/
    // momSeed.SetTheta(TMath::Pi() / 2.0);
    // momSeed.SetTheta(21.3*TMath::Pi()/180.);
    // momSeed.SetTheta(18.96*TMath::Pi()/180.);
    // momSeed.SetPhi(5.*TMath::Pi()/180.);
    trackCand.setCovSeed(covSeed);
    trackCand.setPosMomSeed(posSeed, momSeed, -1);
    trackCand.setPdgCode(fPDGCode);
    // trackCand.Print();

    auto* gfTrack =
        new ((*fGenfitTrackArray)[fGenfitTrackArray->GetEntriesFast()]) genfit::Track(trackCand, *fMeasurementFactory);
    gfTrack->addTrackRep(new genfit::RKTrackRep(fPDGCode));

    auto* trackRep = dynamic_cast<genfit::RKTrackRep*>(gfTrack->getTrackRep(0));

    // Double_t dPVal = 1.E-3;

    try
    {
        fKalmanFitter->processTrackWithRep(gfTrack, trackRep, false);
    }
    catch (genfit::Exception& e)
    {
        std::cout << "   Exception caught from Kalman Fitter : " << e.what() << "\n";
        return nullptr;
    }

    genfit::FitStatus* fitStatus;
    try
    {
        // if (fVerbosity > 0) {
        fitStatus = gfTrack->getFitStatus(trackRep);
        std::cout << cYELLOW << " Is fitted? " << fitStatus->isFitted() << "\n";
        std::cout << " Is Converged ? " << fitStatus->isFitConverged() << "\n";
        std::cout << " Is Converged Partially? " << fitStatus->isFitConvergedPartially() << "\n";
        std::cout << " Is pruned ? " << fitStatus->isTrackPruned() << cNORMAL << "\n";
        fitStatus->Print();
        // }
    }
    catch (genfit::Exception& e)
    {
        return nullptr;
    }

    genfit::MeasuredStateOnPlane fitState;
    // genfit::TrackPoint* firstPoint;
    // genfit::TrackPoint* lastPoint;
    // genfit::KalmanFitterInfo* pointKFitterInfo;
    try
    {
        fitState = gfTrack->getFittedState();
        // if (fVerbosity > 0)
        fitState.Print();
        // Fit result
        fitState.getPosMomCov(pos_res, mom_res, cov_res);
        // if (fVerbosity > 0)
        std::cout << cYELLOW << "    Total Momentum : " << mom_res.Mag() << " - Position : " << pos_res.X() << "  "
                  << pos_res.Y() << "  " << pos_res.Z() << cNORMAL << "\n";
        // firstPoint = gfTrack->getPointWithMeasurement(0);
        // lastPoint  = gfTrack->getPointWithMeasurement(gfTrack->getNumPoints()-1);
        // firstPoint->Print();
        // lastPoint->Print();
        // pointKFitterInfo = firstPoint->getKalmanFitterInfo();
    }
    catch (genfit::Exception& e)
    {
        return nullptr;
    }

    std::cout << " End of GENFIT "
              << "\n";
    std::cout << "               "
              << "\n";

    return gfTrack;
}
