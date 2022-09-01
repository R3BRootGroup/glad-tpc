/******************************************************************************
 *   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2019 Members of R3B Collaboration                          *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU General Public Licence (GPL) version 3,                *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "TClonesArray.h"

#include "R3BGTPC.h"
#include "R3BGTPCHit2Track.h"
#include "R3BGTPCHitData.h"
#include "R3BGTPCTrackData.h"
//#include "R3BGTPCHitPar.h"

#include "dnn.h"
#include "graph.h"
#include "output.h"
#include "pointcloud.h"
#include "option.h"

// R3BGTPCHit2Track: Constructor
R3BGTPCHit2Track::R3BGTPCHit2Track()
    : FairTask("R3B GTPC Hit to Track")
    //    , fHitParams(NULL)
    //, fHit_Par(NULL)
    , fHitCA(NULL)
    , fTrackCA(NULL)
    , fOnline(kFALSE)
{
}

R3BGTPCHit2Track::~R3BGTPCHit2Track()
{
    LOG(INFO) << "R3BGTPCHit2Track: Delete instance";
    if (fHitCA)
        delete fHitCA;
    if (fTrackCA)
        delete fTrackCA;
}

void R3BGTPCHit2Track::SetParContainers()
{
    // Reading GTPCHitPar from FairRuntimeDb
    // FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    // if (!rtdb)
    // {
    //  LOG(ERROR) << "R3BGTPCMapped2Cal:: FairRuntimeDb not opened!";
    // }

    // fHit_Par = (R3BGTPCHitPar*)rtdb->getContainer("GTPCHitPar");
    // if (!fHit_Par)
    // {
    //  LOG(ERROR) << "R3BGTPCCal2Hit::Init() Couldn't get handle on GTPCHitPar container";
    // }
    // else
    // {
    //   LOG(INFO) << "R3BGTPCCal2Hit:: GTPCHitPar container open";
    // }

    fTrackFinder = new R3BGTPCTrackFinder();

}

void R3BGTPCHit2Track::SetParameter()
{
    //--- Parameter Container ---
    /*
          fHitParams = new TArrayF();
          fHitParams = fHit_Par->GetHitParams(); // Array with the Hit parameters
    */
}

InitStatus R3BGTPCHit2Track::Init()
{
    LOG(INFO) << "R3BGTPCHit2Track::Init() ";
    assert(!fTrackCA); // in case someone calls Init() twice.

    // INPUT DATA - Cal
    FairRootManager* ioManager = FairRootManager::Instance();
    if (!ioManager)
        LOG(fatal) << "Init: No FairRootManager";

    fHitCA = (TClonesArray*)ioManager->GetObject("GTPCHitData");
    if (!fHitCA)
        LOG(fatal) << "Init: No GTPCHitData";

    // Register output - Track
    fTrackCA = new TClonesArray("R3BGTPCTrackData", 50);
    if (!fOnline)
    {
        ioManager->Register("GTPCTrackData", "GTPC Track", fTrackCA, kTRUE);
    }
    else
    {
        ioManager->Register("GTPCTrackData", "GTPC Track", fTrackCA, kFALSE);
    }

    SetParameter();
    return kSUCCESS;
}

InitStatus R3BGTPCHit2Track::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}

void R3BGTPCHit2Track::Exec(Option_t* opt)
{
    Reset(); // Reset entries in output arrays, local arrays

    // if (!fTrack_Par)
    // {
    //   LOG(WARNING) << "R3BGTPCHit2Track::NO Container Parameter!!";
    // }

    Opt opt_params;
	int opt_verbose = opt_params.get_verbosity(); 
	PointCloud cloud_xyz;
	fTrackFinder->eventToClusters(fHitCA,cloud_xyz); 
	
	if (cloud_xyz.size() == 0) {
	  std::cerr << "[Error] empty cloud " << std::endl;
	  return;
	  
	}

	if (opt_params.needs_dnn()) {
	  double dnn = std::sqrt(first_quartile(cloud_xyz));
	  if (opt_verbose > 0) {
	    std::cout << "[Info] computed dnn: " << dnn << std::endl;
	  }
	  opt_params.set_dnn(dnn);
	  if (dnn == 0.0) {
	    std::cerr << "[Error] dnn computed as zero. "
		      << "Suggestion: remove doublets, e.g. with 'sort -u'" << std::endl;
	    return;
	  }
	}

        // Step 1) smoothing by position averaging of neighboring points
	PointCloud cloud_xyz_smooth;
	smoothen_cloud(cloud_xyz, cloud_xyz_smooth, opt_params.get_r());

    // Step 2) finding triplets of approximately collinear points
	std::vector<triplet> triplets;
	generate_triplets(cloud_xyz_smooth, triplets, opt_params.get_k(), opt_params.get_n(), opt_params.get_a());

    // Step 3) single link hierarchical clustering of the triplets
    cluster_group cl_group;
    if (cloud_xyz_smooth.size() < 10)
        return;
    compute_hc(cloud_xyz_smooth, cl_group, triplets, opt_params.get_s(), opt_params.get_t(), opt_params.is_tauto(),
		   opt_params.get_dmax(), opt_params.is_dmax(), opt_params.get_linkage(), opt_verbose);

    // Step 4) pruning by removal of small clusters ...
	cleanup_cluster_group(cl_group, opt_params.get_m(), opt_verbose);
	cluster_triplets_to_points(triplets, cl_group);
	// .. and (optionally) by splitting up clusters at gaps > dmax
	if (opt_params.is_dmax()) {
	  cluster_group cleaned_up_cluster_group;
	  for (cluster_group::iterator cl = cl_group.begin(); cl != cl_group.end(); ++cl) {
	    max_step(cleaned_up_cluster_group, *cl, cloud_xyz, opt_params.get_dmax(), opt_params.get_m() + 2);
	  }
	  cl_group = cleaned_up_cluster_group;
	}

	        
        // store cluster labels in points
	add_clusters(cloud_xyz, cl_group, opt_params.is_gnuplot());
    
	// Adapt clusters to AtTrack
    fTrackFinder->clustersToTrack(cloud_xyz, cl_group, fTrackCA, fHitCA);
    return;
}

void R3BGTPCHit2Track::Finish() {}

void R3BGTPCHit2Track::Reset()
{
    LOG(DEBUG) << "Clearing TrackData Structure";
    if (fTrackCA)
        fTrackCA->Clear();
}

//[[deprecated]]
// R3BGTPCTrackData* R3BGTPCHit2Track::AddTrackData(std::size_t trackId, std::vector<R3BGTPCHitData>&
// hitArray,std::vector<R3BGTPCHitData> hitClusterArray)
//{
// It fills the R3BGTPCTrackData
//  TClonesArray& clref = *fTrackCA;
//   Int_t size = clref.GetEntriesFast();
//  return new (clref[size]) R3BGTPCTrackData(trackId, std::move(hitArray),std::move(*hitClusterArray));
//}

ClassImp(R3BGTPCHit2Track)
