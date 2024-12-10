#include "R3BGTPCTrackFinder.h"

#include "TMath.h"
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
#include "option.h"
#include "output.h"
#include "pointcloud.h"

#include <Math/Point3D.h>
#include <Math/Point3Dfwd.h>
#include <Math/Vector3D.h>
#include <Math/Vector3Dfwd.h>
#include <Rtypes.h>
#include <TObject.h>

constexpr auto cRED = "\033[1;31m";
constexpr auto cYELLOW = "\033[1;33m";
constexpr auto cNORMAL = "\033[0m";
constexpr auto cGREEN = "\033[1;32m";

R3BGTPCTrackFinder::R3BGTPCTrackFinder() {}

void R3BGTPCTrackFinder::eventToClusters(TClonesArray* hitCA, PointCloud& cloud)
{

    Int_t nHits = hitCA->GetEntries();
    R3BGTPCHitData** hitData;
    hitData = new R3BGTPCHitData*[nHits];

    if (hitData)
    {

        for (Int_t iHit = 0; iHit < nHits; iHit++)
        {
            Point point;
            hitData[iHit] = (R3BGTPCHitData*)(hitCA->At(iHit));
            point.x = hitData[iHit]->GetX();
            point.y = hitData[iHit]->GetY();
            point.z = hitData[iHit]->GetZ();
            point.SetID(iHit);
            cloud.push_back(point);
        }

        delete hitData;
    }
}

std::unique_ptr<R3BGTPCTrackData> R3BGTPCTrackFinder::clustersToTrack(PointCloud& cloud,
                                                                      const std::vector<cluster_t>& clusters,
                                                                      TClonesArray* trackCA,
                                                                      TClonesArray* hitCA)
{

    std::vector<R3BGTPCTrackData> tracks;
    std::vector<Point> points = cloud;

    for (size_t cluster_index = 0; cluster_index < clusters.size(); ++cluster_index)
    {

        R3BGTPCTrackData track; // One track per cluster

        const std::vector<size_t>& point_indices = clusters[cluster_index];
        if (point_indices.size() == 0)
            continue;

        // add points
        for (std::vector<size_t>::const_iterator it = point_indices.begin(); it != point_indices.end(); ++it)
        {

            const Point& point = cloud[*it];

            Int_t nHits = hitCA->GetEntries();

            R3BGTPCHitData** hitData;
            hitData = new R3BGTPCHitData*[nHits];

            if (hitData)
            {

                hitData[point.GetID()] = (R3BGTPCHitData*)(hitCA->At(point.GetID()));
                track.AddHit(*hitData[point.GetID()]);
                delete hitData;
            }

            // remove current point from vector points
            for (std::vector<Point>::iterator p = points.begin(); p != points.end(); p++)
            {
                if (*p == point)
                {
                    points.erase(p);
                    break;
                }
            }

        } // Point indices

        track.SetTrackId(cluster_index);
        Clusterize(track, 0.70, 1.5);

        tracks.push_back(track);

    } // Clusters loop

    std::cout << cRED << " Tracks found " << tracks.size() << cNORMAL << "\n";

    // TODO
    // Dump noise into pattern event
    // auto retEvent = std::make_unique<AtPatternEvent>();
    // for (const auto &point : points)
    // retEvent->AddNoise(event.GetHit(point.intensity));

    for (auto& track : tracks)
    {
        // if (track.GetHitArray().size() > 0)
        // SetTrackInitialParameters(track);

        TClonesArray& clref = *trackCA;
        Int_t size = clref.GetEntriesFast();
        new (clref[size]) R3BGTPCTrackData(
            track.GetTrackId(), std::move(track.GetHitArray()), std::move(*track.GetHitClusterArray()));
    }

    return NULL;
}

void R3BGTPCTrackFinder::Clusterize(R3BGTPCTrackData& track, Float_t distance, Float_t radius)
{

    std::vector<R3BGTPCHitData> hitArray = track.GetHitArray();
    std::vector<R3BGTPCHitData> hitTBArray;
    int clusterID = 0;

    std::cout << " Number of hits per track : " << hitArray.size() << "\n";

    if (hitArray.size() > 0)
    {

        ROOT::Math::XYZVector refPos{ hitArray.at(0).GetX(), hitArray.at(0).GetY(), hitArray.at(0).GetZ() };

        for (auto iHit = 0; iHit < hitArray.size(); ++iHit)
        {

            auto hit = hitArray.at(iHit);
            ROOT::Math::XYZVector hitPos{ hit.GetX(), hit.GetY(), hit.GetZ() };

            // Check distance with respect to reference Hit
            Double_t distRef = TMath::Sqrt((hitPos - refPos).Mag2());

            if (distRef < distance)
            {

                continue;
            }
            else
            {

                Double_t clusterQ = 0.0;
                hitTBArray.clear();
                std::copy_if(hitArray.begin(),
                             hitArray.end(),
                             std::back_inserter(hitTBArray),
                             [&refPos, radius](R3BGTPCHitData& hitIn)
                             {
                                 ROOT::Math::XYZVector hitInPos{ hitIn.GetX(), hitIn.GetY(), hitIn.GetZ() };
                                 return TMath::Sqrt((hitInPos - refPos).Mag2()) < radius;
                             });

                if (hitTBArray.size() > 0)
                {
                    double x = 0, y = 0, z = 0;

                    int timeStamp = 0;
                    std::shared_ptr<R3BGTPCHitClusterData> hitCluster = std::make_shared<R3BGTPCHitClusterData>();
                    hitCluster->SetClusterID(clusterID);
                    Double_t hitQ = 0.0;
                    std::for_each(hitTBArray.begin(),
                                  hitTBArray.end(),
                                  [&x, &y, &z, &hitQ, &timeStamp](R3BGTPCHitData& hitInQ)
                                  {
                                      ROOT::Math::XYZPoint pos{ hitInQ.GetX(), hitInQ.GetY(), hitInQ.GetZ() };
                                      x += pos.X() * hitInQ.GetEnergy();
                                      y += pos.Y() * hitInQ.GetEnergy();
                                      z += pos.Z();
                                      hitQ += hitInQ.GetEnergy();
                                      // TODO
                                      // timeStamp += hitInQ.GetTimeStamp();
                                  });
                    x /= hitQ;
                    y /= hitQ;
                    z /= hitTBArray.size();
                    // timeStamp /= hitTBArray.size();

                    ROOT::Math::XYZPoint clustPos(x, y, z);
                    Bool_t checkDistance = kTRUE;

                    // Check distance with respect to existing clusters
                    for (auto iClusterHit : *track.GetHitClusterArray())
                    {
                        ROOT::Math::XYZPoint iclusterHitPos{ iClusterHit.GetX(),
                                                             iClusterHit.GetY(),
                                                             iClusterHit.GetZ() };
                        if (TMath::Sqrt((iclusterHitPos - clustPos).Mag2()) < distance)
                        {
                            // std::cout<<" Cluster with less than  : "<<distance<<" found
                            // "<<"\n";
                            checkDistance = kFALSE;
                            continue;
                        }
                    }

                    if (checkDistance)
                    {

                        hitCluster->SetEnergy(hitQ);
                        hitCluster->SetX(x);
                        hitCluster->SetY(y);
                        hitCluster->SetZ(z);
                        // hitCluster->SetTime(timeStamp);
                        ++clusterID;
                        track.AddClusterHit(hitCluster);
                    }

                } // if hitTBArray>0

            } // if distance

            ROOT::Math::XYZVector refPosBuff{ hitArray.at(iHit).GetX(),
                                              hitArray.at(iHit).GetY(),
                                              hitArray.at(iHit).GetZ() };

            refPos = refPosBuff;

        } // for Hit array

    } // if array size
}
