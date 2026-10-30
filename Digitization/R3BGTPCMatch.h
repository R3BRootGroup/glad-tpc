/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 ******************************************************************************/

#pragma once

#include "FairLink.h"
#include "TObject.h"

#include <iostream>
#include <vector>

/**
 * R3BGTPCLink
 * ===========
 *
 * One MC-truth link: where a part of a signal came from, as FairLink (FairRoot) and
 * CbmLink (CbmRoot):
 *
 *   file   : input file id of the MC event (FairEventHeader::GetInputFileId)
 *   entry  : MC event = entry in that file (FairEventHeader::GetMCEntryNumber)
 *   index  : MC track index (position in the MCTrack TClonesArray of that entry)
 *   weight : signal of that track [ADC counts]
 *
 * file and entry are always real values (>= 0). Unlike FairLink, -1 is never used: in
 * FairLink, -1 means "the current file / entry", not "no MC origin". Electronics noise
 * has no MC origin, so it is NOT a link (see R3BGTPCMatch::GetNoiseWeight).
 * The type of FairLink (branch id) is not stored: an R3BGTPCLink always points to MCTrack.
 * ToFairLink() makes the FairLink, for FairRoot tools.
 */
class R3BGTPCLink
{
  public:
    R3BGTPCLink() = default;
    R3BGTPCLink(Int_t file, Int_t entry, Int_t index, Double_t weight)
        : fFile(file)
        , fEntry(entry)
        , fIndex(index)
        , fWeight(weight)
    {
    }

    Int_t GetFile() const { return fFile; }
    Int_t GetEntry() const { return fEntry; }
    Int_t GetIndex() const { return fIndex; } // MC track index
    Double_t GetWeight() const { return fWeight; }
    void AddWeight(Double_t w) { fWeight += w; }

    /** same MC track of the same MC event (the weight is not compared) */
    bool operator==(const R3BGTPCLink& o) const { return fFile == o.fFile && fEntry == o.fEntry && fIndex == o.fIndex; }
    /** ordered by file, entry, index, as FairLink */
    bool operator<(const R3BGTPCLink& o) const
    {
        if (fFile != o.fFile)
            return fFile < o.fFile;
        if (fEntry != o.fEntry)
            return fEntry < o.fEntry;
        return fIndex < o.fIndex;
    }

    /** FairLink(file, entry, type, index, weight); type = branch id of MCTrack in the current run
     *  (FairRootManager::GetBranchId("MCTrack")) */
    FairLink ToFairLink(Int_t mcTrackType) const
    {
        return FairLink(fFile, fEntry, mcTrackType, fIndex, static_cast<Float_t>(fWeight));
    }

    /** "(file/entry/index/weight)", the format of FairLink::PrintLinkInfo without the type */
    void PrintLinkInfo(std::ostream& out = std::cout) const
    {
        out << "(" << fFile << "/" << fEntry << "/" << fIndex << "/" << fWeight << ")";
    }

  private:
    Int_t fFile = 0;
    Int_t fEntry = 0;
    Int_t fIndex = 0;
    Double_t fWeight = 0.;

    ClassDefNV(R3BGTPCLink, 1)
};

/**
 * R3BGTPCMatch
 * ============
 *
 * MC truth of one reconstructed object (digi, cluster, hit or track), as CbmMatch
 * (CbmRoot) / FairMultiLinkedData (FairRoot):
 *
 *   links        : the MC tracks that made the signal (R3BGTPCLink), one per
 *                  (file, entry, track), weights added up
 *   noise weight : the part of the signal that the electronics noise made alone
 *                  [ADC counts]; no MC origin, so it is not a link
 *
 * Digi (R3BGTPCVMMDigitizer):
 *   signal digi          : links = share of every MC track in the noiseless pulse height at
 *                          the peak time (linear shaper). The noise ON the pulse is not stored
 *                          (it can be negative): ADC - GetMCWeight() = noise + ADC rounding.
 *   noise digi           : no links, noise weight = its amplitude (R3BGTPCVMMDigi::IsNoise)
 *   neighbour, no charge : no links, noise weight = its amplitude
 * Cluster / hit / track: the matches of their digis / cluster / hits added up (Add).
 *
 * The branches GTPCVMMDigiMatch, GTPCVMMClusterMatch, GTPCVMMHitMatch and GTPCVMMTrackMatch
 * have one entry per object, at the same index as GTPCVMMDigi, ... GTPCVMMTrack. The
 * object's own position (FairMultiLinkedData::GetEntryNr) is therefore not stored, and the
 * reconstruction history is in the objects (cluster -> digi indices, hit -> cluster index,
 * track -> hit indices).
 */
class R3BGTPCMatch : public TObject
{
  public:
    R3BGTPCMatch() = default;
    ~R3BGTPCMatch() override = default;

    // ---- filling
    /** add weight to the link (file, entry, MC track), created if new */
    void AddLink(Int_t file, Int_t entry, Int_t mcTrack, Double_t weight);
    void AddLink(const R3BGTPCLink& link)
    {
        AddLink(link.GetFile(), link.GetEntry(), link.GetIndex(), link.GetWeight());
    }
    /** add to the noise weight */
    void AddNoise(Double_t weight) { fNoiseWeight += weight; }
    /** add all links and the noise of another match, multiplied by scale */
    void Add(const R3BGTPCMatch& other, Double_t scale = 1.);
    void Clear(Option_t* = "") override
    {
        fLinks.clear();
        fNoiseWeight = 0.;
    }

    // ---- links
    Int_t GetNumberOfLinks() const { return static_cast<Int_t>(fLinks.size()); }
    const R3BGTPCLink& GetLink(Int_t i) const { return fLinks[i]; }
    const std::vector<R3BGTPCLink>& GetLinks() const { return fLinks; }
    /** links sorted by weight, largest first (as FairMultiLinkedData::GetSortedMCTracks) */
    std::vector<R3BGTPCLink> GetSortedLinks() const;

    // ---- signal and noise
    Double_t GetMCWeight() const; // sum of the links
    Double_t GetNoiseWeight() const { return fNoiseWeight; }
    Double_t GetTotalWeight() const { return GetMCWeight() + fNoiseWeight; }
    Bool_t HasMC() const { return !fLinks.empty(); }
    Bool_t HasNoise() const { return fNoiseWeight > 0.; }
    /** made by noise alone: no MC link (a neighbour pad without charge, a noise digi, or a
     *  cluster / hit / track of those only) */
    Bool_t IsNoise() const { return fLinks.empty() && fNoiseWeight > 0.; }

    /** weight of one MC track of one MC event, 0 if not linked */
    Double_t GetWeight(Int_t file, Int_t entry, Int_t mcTrack) const;
    /** weight of an MC track index summed over all files and entries: unambiguous only when the
     *  objects of one event come from one MC event (event-by-event simulation) */
    Double_t GetWeight(Int_t mcTrack) const;

    // ---- best link (the noise is never matched)
    /** position of the link with the largest weight, -1 without links */
    Int_t GetMatchedLinkPos() const;
    /** the link with the largest weight; check HasMC() first (a default link otherwise) */
    R3BGTPCLink GetMatchedLink() const;
    Int_t GetMatchedTrack() const; // -1: no MC link
    Int_t GetMatchedEntry() const; // -1: no MC link
    Int_t GetMatchedFile() const;  // -1: no MC link
    /** matched weight / total weight (noise included), 0 without MC link */
    Double_t GetPurity() const;

    void Print(const Option_t* option = "") const override;

  private:
    std::vector<R3BGTPCLink> fLinks; // MC links
    Double_t fNoiseWeight = 0.;      // [ADC counts], made by the electronics noise alone

  public:
    ClassDefOverride(R3BGTPCMatch, 3) // 3: R3BGTPCLink (file, entry, track, weight), noise as a weight
};
