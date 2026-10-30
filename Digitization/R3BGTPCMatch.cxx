#include "R3BGTPCMatch.h"

#include <algorithm>

void R3BGTPCMatch::AddLink(Int_t file, Int_t entry, Int_t mcTrack, Double_t weight)
{
    const R3BGTPCLink link(file, entry, mcTrack, weight);
    const auto it = std::find(fLinks.begin(), fLinks.end(), link); // same file, entry, track
    if (it != fLinks.end())
        it->AddWeight(weight);
    else
        fLinks.push_back(link);
}

void R3BGTPCMatch::Add(const R3BGTPCMatch& other, Double_t scale)
{
    for (const R3BGTPCLink& link : other.fLinks)
        AddLink(link.GetFile(), link.GetEntry(), link.GetIndex(), scale * link.GetWeight());
    fNoiseWeight += scale * other.fNoiseWeight;
}

std::vector<R3BGTPCLink> R3BGTPCMatch::GetSortedLinks() const
{
    std::vector<R3BGTPCLink> sorted = fLinks;
    std::stable_sort(sorted.begin(),
                     sorted.end(),
                     [](const R3BGTPCLink& a, const R3BGTPCLink& b) { return a.GetWeight() > b.GetWeight(); });
    return sorted;
}

Double_t R3BGTPCMatch::GetMCWeight() const
{
    Double_t sum = 0.;
    for (const R3BGTPCLink& link : fLinks)
        sum += link.GetWeight();
    return sum;
}

Double_t R3BGTPCMatch::GetWeight(Int_t file, Int_t entry, Int_t mcTrack) const
{
    const auto it = std::find(fLinks.begin(), fLinks.end(), R3BGTPCLink(file, entry, mcTrack, 0.));
    return it != fLinks.end() ? it->GetWeight() : 0.;
}

Double_t R3BGTPCMatch::GetWeight(Int_t mcTrack) const
{
    Double_t sum = 0.;
    for (const R3BGTPCLink& link : fLinks)
        if (link.GetIndex() == mcTrack)
            sum += link.GetWeight();
    return sum;
}

Int_t R3BGTPCMatch::GetMatchedLinkPos() const
{
    Int_t best = -1;
    for (Int_t i = 0; i < GetNumberOfLinks(); ++i)
        if (best < 0 || fLinks[i].GetWeight() > fLinks[best].GetWeight())
            best = i;
    return best;
}

R3BGTPCLink R3BGTPCMatch::GetMatchedLink() const
{
    const Int_t i = GetMatchedLinkPos();
    return i < 0 ? R3BGTPCLink() : fLinks[i];
}

Int_t R3BGTPCMatch::GetMatchedTrack() const
{
    const Int_t i = GetMatchedLinkPos();
    return i < 0 ? -1 : fLinks[i].GetIndex();
}

Int_t R3BGTPCMatch::GetMatchedEntry() const
{
    const Int_t i = GetMatchedLinkPos();
    return i < 0 ? -1 : fLinks[i].GetEntry();
}

Int_t R3BGTPCMatch::GetMatchedFile() const
{
    const Int_t i = GetMatchedLinkPos();
    return i < 0 ? -1 : fLinks[i].GetFile();
}

Double_t R3BGTPCMatch::GetPurity() const
{
    const Int_t i = GetMatchedLinkPos();
    const Double_t total = GetTotalWeight();
    return (i >= 0 && total > 0.) ? fLinks[i].GetWeight() / total : 0.;
}

void R3BGTPCMatch::Print(const Option_t*) const
{
    std::cout << "R3BGTPCMatch  links (file/entry/track/ADC):";
    for (const R3BGTPCLink& link : GetSortedLinks())
    {
        std::cout << " ";
        link.PrintLinkInfo(std::cout);
    }
    std::cout << "  noise " << fNoiseWeight << " ADC  -> matched track " << GetMatchedTrack() << " (entry "
              << GetMatchedEntry() << ", file " << GetMatchedFile() << "), purity " << GetPurity()
              << (IsNoise() ? "  NOISE ONLY" : "") << std::endl;
}

ClassImp(R3BGTPCMatch)
