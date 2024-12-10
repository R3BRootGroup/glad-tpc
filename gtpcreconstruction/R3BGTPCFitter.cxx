#include "R3BGTPCFitter.h"
#include "R3BGTPCHitClusterData.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCSpacePointMeasurement.h"
#include "R3BGTPCTrackData.h"

// GENFIT
#include <AbsKalmanFitter.h>
#include <AbsMeasurement.h>
#include <AbsTrackRep.h>
#include <ConstField.h>
#include <Exception.h>
#include <FieldManager.h>
#include <FitStatus.h>
#include <KalmanFitterRefTrack.h>
#include <MaterialEffects.h>
#include <MeasuredStateOnPlane.h>
#include <MeasurementFactory.h>
#include <MeasurementProducer.h>
#include <Track.h>

#include <Math/Vector3D.h>
#include <Math/Vector3Dfwd.h>
#include <Rtypes.h>

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

    fGenfitTrackArray = new TClonesArray("genfit::Track");
    fHitClusterArray = new TClonesArray("R3BGTPCHitClusterData");

    fMeasurementProducer =
        new genfit::MeasurementProducer<R3BGTPCHitClusterData, genfit::R3BGTPCSpacepointMeasurement>(fHitClusterArray);
    fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
    fMeasurementFactory->addProducer(fTPCDetID, fMeasurementProducer);

    genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.0, 20.0, 0.0)); // kGauss
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

genfit::Track* R3BGTPCFitter::FitTrack(R3BGTPCTrackData* track)
{
    fHitClusterArray->Delete();
    genfit::TrackCand trackCand;

    auto hitClusterArray = track->GetHitClusterArray();

    TVector3 pos_res;
    TVector3 mom_res;
    TMatrixDSym cov_res;

    std::cout << cYELLOW << " Track " << track->GetTrackId() << " with " << hitClusterArray->size() << " clusters "
              << cNORMAL << "\n";

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
    // std::cout<<" End cluster "<<endCluster.GetX()<<"  "<<endCluster.GetY()<<"
    // "<<endCluster.GetZ()<<"\n";

    TVector3 posSeed(iniCluster.GetX(), iniCluster.GetY(), iniCluster.GetZ());
    posSeed.SetMag(posSeed.Mag());

    TMatrixDSym covSeed(6); // TODO  (hard coded in SpacePoint measurement)
    TMatrixD covMatrix = iniCluster.GetCovMatrix();
    for (Int_t iComp = 0; iComp < 3; iComp++)
    {
        covSeed(iComp, iComp) = 0.4; // covMatrix(iComp, iComp); // units to check
        std::cout << covSeed(iComp, iComp) << "\n";
    }

    for (Int_t iComp = 3; iComp < 6; iComp++)
        covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

    TVector3 momSeed(0.0, 0.0, 0.1);
    momSeed.SetTheta(TMath::Pi() / 2.0);
    // momSeed.SetPhi(0);
    trackCand.setCovSeed(covSeed);
    trackCand.setPosMomSeed(posSeed, momSeed, -1);
    trackCand.setPdgCode(fPDGCode);
    // trackCand.Print();

    auto* gfTrack =
        new ((*fGenfitTrackArray)[fGenfitTrackArray->GetEntriesFast()]) genfit::Track(trackCand, *fMeasurementFactory);
    gfTrack->addTrackRep(new genfit::RKTrackRep(fPDGCode));

    auto* trackRep = dynamic_cast<genfit::RKTrackRep*>(gfTrack->getTrackRep(0));

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
