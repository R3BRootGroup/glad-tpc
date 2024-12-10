/******************************************************************************
 *   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2018-2025 Members of R3B Collaboration                     *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU Lesser General Public Licence (LGPL) version 3,        *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#pragma once

#include "R3BDetector.h"
#include "TLorentzVector.h"

class TClonesArray;
class R3BGTPCPoint;
class FairVolume;

class R3BGTPC : public R3BDetector
{

  public:
    /** Default constructor **/
    R3BGTPC();

    /** Standard constructor.
     *@param geoFile name of the ROOT geometry file
     *@param trans   position
     *@param rot     rotation
     */
    R3BGTPC(const TString& geoFile, const TGeoTranslation& trans, const TGeoRotation& rot = TGeoRotation());

    /** Standard constructor.
     *@param geoFile name of the ROOT geometry file
     *@param combi   position + rotation
     */
    R3BGTPC(const TString& geoFile, const TGeoCombiTrans& combi = TGeoCombiTrans());

    /** Destructor **/
    virtual ~R3BGTPC();

    /**Initialize the the detectors**/
    virtual void Initialize();

    /** Virtual method ProcessHits
     **
     ** Defines the action to be taken when a step is inside the
     ** active volume. Creates a R3BGTPCPoint and adds it to the
     ** collection.
     *@param vol  Pointer to the active volume
     **/
    virtual Bool_t ProcessHits(FairVolume* vol = 0);

    /** Virtual method BeginEvent
     **
     ** If verbosity level is set, print hit collection at the
     ** end of the event and resets it afterwards.
     **/
    virtual void BeginEvent();

    /** Virtual method EndOfEvent
     **
     ** Added support for R3BRawHit
     ** If verbosity level is set, print hit collection at the
     ** end of the event and resets it afterwards.
     **/
    virtual void EndOfEvent();

    /** Virtual method Register
     **
     ** Registers the hit collection in the ROOT manager.
     **/
    virtual void Register();

    /** Accessor to the hit collection **/
    virtual TClonesArray* GetCollection(Int_t iColl) const;

    /** Virtual method Print
     **
     ** Screen output of hit collection.
     **/
    virtual void Print(Option_t* option = "") const;

    /** Virtual method Reset
     **
     ** Clears the hit collection
     **/
    virtual void Reset();

    virtual void FinishRun();

    virtual Bool_t CheckIfSensitive(std::string name);

    virtual void SetSpecialPhysicsCuts();

    Int_t GetTrackStatus(bool NewTrack,
                         bool TrackDisappeared,
                         bool TrackStop,
                         bool TrackAlive,
                         bool TrackEntering,
                         bool TrackExiting,
                         bool TrackInside,
                         bool TrackOut);

  private:
    TClonesArray* fGTPCPointCollection;

    ClassDef(R3BGTPC, 1);
};
