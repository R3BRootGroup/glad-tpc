#ifndef R3BGTPCTRACKFINDER_H
#define R3BGTPCTRACKFINDER_H

#include "TClonesArray.h"
#include <Rtypes.h> // for THashConsistencyHolder, ClassDef

#include "cluster.h" // for Cluster
#include <stdio.h>   // for size_t

#include <memory> // for unique_ptr
#include <vector> // for vector

#include "R3BGTPCHitClusterData.h"
#include "R3BGTPCTrackData.h"

struct tc_params
{
    float s;
    size_t k;
    size_t n;
    size_t m;
    float r;
    float a;
    float t;
    float _padding;
};

class R3BGTPCTrackFinder
{
  private:
    tc_params inputParams{ .s = 0.3, .k = 19, .n = 2, .m = 15, .r = 2, .a = 0.03, .t = 4.0 };

  public:
    R3BGTPCTrackFinder();
    virtual ~R3BGTPCTrackFinder() = default;
    void Clusterize(R3BGTPCTrackData& track, Float_t distance, Float_t radius);
    void eventToClusters(TClonesArray* hitCA, PointCloud& cloud);
    std::unique_ptr<R3BGTPCTrackData> clustersToTrack(PointCloud& cloud,
                                                      const std::vector<cluster_t>& clusters,
                                                      TClonesArray* trackCA,
                                                      TClonesArray* hitCA);

    void SetScluster(float s) { inputParams.s = s; }
    void SetKtriplet(size_t k) { inputParams.k = k; }
    void SetNtriplet(size_t n) { inputParams.n = n; }
    void SetMcluster(size_t m) { inputParams.m = m; }
    void SetRsmooth(float r) { inputParams.r = r; }
    void SetAtriplet(float a) { inputParams.a = a; }
    void SetTcluster(float t) { inputParams.t = t; }

    ClassDef(R3BGTPCTrackFinder, 1);
};

#endif
