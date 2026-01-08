
/*##############################################################################
# Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH #
#   Copyright (C) 2018-2026 Members of the R3B Collaboration                 #
#                                                                            #
#            This software is distributed under the terms of the             #
#              GNU Lesser General Public Licence (GPL) version 3,            #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################*/

#pragma once

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
