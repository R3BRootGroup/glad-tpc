#include "R3BGTPCTrackFinder.h"

#include <Math/Point3D.h> // for PositionVector3D

#include <boost/core/checked_delete.hpp>  // for checked_delete
#include <boost/smart_ptr/shared_ptr.hpp> // for shared_ptr

#include <algorithm>
#include <cmath>    // for sqrt
#include <iostream> // for cout, cerr
#include <memory>   // for allocator_traits<>::value_...
#include <utility>  // for move

#include "dnn.h"
#include "graph.h"
#include "output.h"
#include "pointcloud.h"
#include "option.h"

R3BGTPCTrackFinder::R3BGTPCTrackFinder()
{
}

std::unique_ptr<R3BGTPCTrackData> R3BGTPCTrackFinder::FindTracks(R3BGTPCHitData** hitData)
{

    // TODO
    return NULL;
}

void R3BGTPCTrackFinder::eventToClusters(TClonesArray* hitCA, PointCloud& cloud)
{

    Int_t nHits = hitCA->GetEntries();
    R3BGTPCHitData** hitData;
    hitData = new R3BGTPCHitData* [nHits];

    if (hitData)
    {

        for (Int_t iHit = 0; iHit < nHits; iHit++)
        {
            Point point;
            /*const AtHit hit = event.GetHit(iHit);
          auto position = hit.GetPosition();
          point.x = position.X();
          point.y = position.Y();
          point.z = position.Z();
          point.SetID(iHit);*/
            cloud.push_back(point);
        }

        delete hitData;
    }
}
