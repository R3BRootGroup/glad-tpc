/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * SPDX-License-Identifier: LGPL-3.0-or-later
 ******************************************************************************/

#include "R3BGTPCMap.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
    constexpr Double_t kTol = 1.e-9;

    Bool_t Positive(Double_t x) { return std::isfinite(x) && x > 0.; }

    Bool_t NonNegative(Double_t x) { return std::isfinite(x) && x >= 0.; }
} // namespace

R3BGTPCMap::R3BGTPCMap()
    : fPadCoord(boost::extents[1][4][2])
{
}

R3BGTPCMap::~R3BGTPCMap()
{
    delete fPadPlane;
    fPadPlane = nullptr;
}

Bool_t R3BGTPCMap::IsConfigured() const
{
    return fNumberOfRowsX > 0 && fNumberOfColumnsZ > 0 && Positive(fPadSizeXcm) && Positive(fPadSizeZcm) &&
           NonNegative(fGapXcm) && NonNegative(fGapZcm) && Positive(fSizeXcm) && Positive(fSizeZcm);
}

Double_t R3BGTPCMap::GetGridSpanXcm() const
{
    if (fNumberOfRowsX <= 0)
        return 0.;

    return static_cast<Double_t>(fNumberOfRowsX) * fPadSizeXcm + static_cast<Double_t>(fNumberOfRowsX - 1) * fGapXcm;
}

Double_t R3BGTPCMap::GetGridSpanZcm() const
{
    if (fNumberOfColumnsZ <= 0)
        return 0.;

    return static_cast<Double_t>(fNumberOfColumnsZ) * fPadSizeZcm +
           static_cast<Double_t>(fNumberOfColumnsZ - 1) * fGapZcm;
}

Double_t R3BGTPCMap::GetGridOffsetXcm() const
{
    const Double_t rest = fSizeXcm - GetGridSpanXcm();
    return rest > 0. ? 0.5 * rest : 0.;
}

Double_t R3BGTPCMap::GetGridOffsetZcm() const
{
    const Double_t rest = fSizeZcm - GetGridSpanZcm();
    return rest > 0. ? 0.5 * rest : 0.;
}

Bool_t R3BGTPCMap::ConfigurePadGeometry(Double_t sizeXcm,
                                        Double_t sizeZcm,
                                        Int_t rowsX,
                                        Int_t columnsZ,
                                        Double_t padSizeXcm,
                                        Double_t padSizeZcm,
                                        Double_t gapXcm,
                                        Double_t gapZcm)
{
    if (!Positive(sizeXcm) || !Positive(sizeZcm) || !Positive(padSizeXcm) || !Positive(padSizeZcm) ||
        !NonNegative(gapXcm) || !NonNegative(gapZcm) || rowsX <= 0 || columnsZ <= 0)
    {
        std::cerr << "R3BGTPCMap::ConfigurePadGeometry: invalid geometry input\n";
        return kFALSE;
    }

    const Long64_t nPads = static_cast<Long64_t>(rowsX) * static_cast<Long64_t>(columnsZ);

    if (nPads <= 0 || nPads > std::numeric_limits<UShort_t>::max())
    {
        std::cerr << "R3BGTPCMap::ConfigurePadGeometry: pad count " << nPads << " exceeds supported UShort_t range\n";
        return kFALSE;
    }

    const Double_t spanX = rowsX * padSizeXcm + (rowsX - 1) * gapXcm;
    const Double_t spanZ = columnsZ * padSizeZcm + (columnsZ - 1) * gapZcm;

    if (spanX > sizeXcm + kTol || spanZ > sizeZcm + kTol)
    {
        std::cerr << "R3BGTPCMap::ConfigurePadGeometry: pad matrix does not fit active face\n"
                  << "  X: span=" << spanX << " cm, active=" << sizeXcm << " cm\n"
                  << "  Z: span=" << spanZ << " cm, active=" << sizeZcm << " cm\n";
        return kFALSE;
    }

    fSizeXcm = sizeXcm;
    fSizeZcm = sizeZcm;
    fNumberOfRowsX = rowsX;
    fNumberOfColumnsZ = columnsZ;
    fPadSizeXcm = padSizeXcm;
    fPadSizeZcm = padSizeZcm;
    fGapXcm = gapXcm;
    fGapZcm = gapZcm;

    fPadCoord.resize(boost::extents[GetNumberOfPads()][4][2]);
    std::fill(fPadCoord.data(), fPadCoord.data() + fPadCoord.num_elements(), 0.);

    delete fPadPlane;
    fPadPlane = nullptr;

    return kTRUE;
}

void R3BGTPCMap::GeneratePadPlane()
{
    if (!IsConfigured())
    {
        std::cerr << "R3BGTPCMap::GeneratePadPlane: map is not configured\n";
        return;
    }

    delete fPadPlane;
    fPadPlane = new TH2Poly();
    fPadPlane->SetDirectory(nullptr);
    fPadPlane->SetName("GTPCVirtualPadPlane");
    fPadPlane->SetTitle("GTPC virtual pad plane;local Z-Z_{min} [cm];local X-X_{min} [cm]");

    const Double_t pitchX = GetPitchXcm();
    const Double_t pitchZ = GetPitchZcm();
    const Double_t offsetX = GetGridOffsetXcm();
    const Double_t offsetZ = GetGridOffsetZcm();

    for (Int_t columnZ = 0; columnZ < fNumberOfColumnsZ; ++columnZ)
    {
        const Double_t z0 = offsetZ + columnZ * pitchZ;
        const Double_t z1 = z0 + fPadSizeZcm;

        for (Int_t rowX = 0; rowX < fNumberOfRowsX; ++rowX)
        {
            const Int_t padId = columnZ * fNumberOfRowsX + rowX;
            const Double_t x0 = offsetX + rowX * pitchX;
            const Double_t x1 = x0 + fPadSizeXcm;

            fPadCoord[padId][0][0] = z0;
            fPadCoord[padId][0][1] = x0;
            fPadCoord[padId][1][0] = z0;
            fPadCoord[padId][1][1] = x1;
            fPadCoord[padId][2][0] = z1;
            fPadCoord[padId][2][1] = x1;
            fPadCoord[padId][3][0] = z1;
            fPadCoord[padId][3][1] = x0;

            Double_t z[5] = { z0, z0, z1, z1, z0 };
            Double_t x[5] = { x0, x1, x1, x0, x0 };
            fPadPlane->AddBin(5, z, x);
        }
    }

    fPadPlane->ChangePartition(500, 500);

    std::cout << "R3BGTPCMap: " << fNumberOfRowsX << " x " << fNumberOfColumnsZ << " = " << GetNumberOfPads()
              << " pads\n"
              << "  active X/Z         = " << fSizeXcm << " / " << fSizeZcm << " cm\n"
              << "  pad X/Z            = " << fPadSizeXcm << " / " << fPadSizeZcm << " cm\n"
              << "  gap X/Z            = " << fGapXcm << " / " << fGapZcm << " cm\n"
              << "  pitch X/Z          = " << pitchX << " / " << pitchZ << " cm\n"
              << "  physical span X/Z  = " << GetGridSpanXcm() << " / " << GetGridSpanZcm() << " cm\n"
              << "  outer margin X/Z   = " << GetGridOffsetXcm() << " / " << GetGridOffsetZcm() << " cm per side\n";
}

Int_t R3BGTPCMap::FindPad(Double_t planeZcm, Double_t planeXcm) const
{
    if (!IsConfigured() || !std::isfinite(planeXcm) || !std::isfinite(planeZcm))
        return -1;

    if (planeXcm < 0. || planeXcm >= fSizeXcm || planeZcm < 0. || planeZcm >= fSizeZcm)
        return -1;

    const Double_t offX = GetGridOffsetXcm();
    const Double_t offZ = GetGridOffsetZcm();
    const Double_t spanX = GetGridSpanXcm();
    const Double_t spanZ = GetGridSpanZcm();

    if (planeXcm < offX || planeXcm >= offX + spanX || planeZcm < offZ || planeZcm >= offZ + spanZ)
        return -2;

    const Double_t x = planeXcm - offX;
    const Double_t z = planeZcm - offZ;
    const Double_t pitchX = GetPitchXcm();
    const Double_t pitchZ = GetPitchZcm();

    Int_t rowX = static_cast<Int_t>(std::floor(x / pitchX));
    Int_t colZ = static_cast<Int_t>(std::floor(z / pitchZ));

    // The physical span ends at the last pad edge, so numerical points exactly
    // near the final edge may floor to N. Clamp only this round-off case.
    rowX = std::min(rowX, fNumberOfRowsX - 1);
    colZ = std::min(colZ, fNumberOfColumnsZ - 1);

    if (rowX < 0 || colZ < 0)
        return -2;

    const Double_t xWithin = x - rowX * pitchX;
    const Double_t zWithin = z - colZ * pitchZ;

    // Interior gap: assign to nearest neighbouring pad. This is a projection-
    // level approximation; later charge sharing belongs in the amplification/
    // resistive/readout digitisation model.
    if (rowX < fNumberOfRowsX - 1 && xWithin >= fPadSizeXcm)
    {
        if (xWithin - fPadSizeXcm >= 0.5 * fGapXcm)
            ++rowX;
    }

    if (colZ < fNumberOfColumnsZ - 1 && zWithin >= fPadSizeZcm)
    {
        if (zWithin - fPadSizeZcm >= 0.5 * fGapZcm)
            ++colZ;
    }

    return colZ * fNumberOfRowsX + rowX;
}

Int_t R3BGTPCMap::BinToPad(Int_t binval) const
{
    return (binval >= 1 && binval <= GetNumberOfPads()) ? binval - 1 : -1;
}

std::vector<Float_t> R3BGTPCMap::CalcPadCenter(Int_t padId) const
{
    if (!IsConfigured() || padId < 0 || padId >= GetNumberOfPads())
        return { -9999.F, -9999.F };

    const Float_t z = static_cast<Float_t>(0.5 * (fPadCoord[padId][0][0] + fPadCoord[padId][2][0]));
    const Float_t x = static_cast<Float_t>(0.5 * (fPadCoord[padId][0][1] + fPadCoord[padId][1][1]));

    return { z, x };
}

TH2Poly* R3BGTPCMap::GetPadPlane() { return fPadPlane; }

ClassImp(R3BGTPCMap)
