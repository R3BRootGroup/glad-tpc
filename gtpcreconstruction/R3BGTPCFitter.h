#ifndef R3BGTPCFITTER_H
#define R3BGTPCFITTER_H

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"
#define cGREEN "\033[1;32m"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "TClonesArray.h"

class R3BGTPCTrackData;
class R3BGTPCHitClusterData;

namespace genfit
{
    class Track;
    class AbsKalmanFitter;
    class AbsMeasurement;
    class R3BGTPCSpacepointMeasurement;
    template <class hit_T, class measurement_T>
    class MeasurementProducer;
    template <class measurement_T>
    class MeasurementFactory;
} // namespace genfit

class R3BGTPCFitter
{

  public:
    R3BGTPCFitter();
    ~R3BGTPCFitter();

    void Init();
    genfit::Track* FitTrack(R3BGTPCTrackData* track);

  private:
    Int_t fTPCDetID{ 0 };
    Int_t fPDGCode{ 211 };

    TClonesArray* fHitClusterArray;
    TClonesArray* fGenfitTrackArray;

    std::shared_ptr<genfit::AbsKalmanFitter> fKalmanFitter;

    genfit::MeasurementProducer<R3BGTPCHitClusterData, genfit::R3BGTPCSpacepointMeasurement>* fMeasurementProducer;
    genfit::MeasurementFactory<genfit::AbsMeasurement>* fMeasurementFactory;
};

#endif
