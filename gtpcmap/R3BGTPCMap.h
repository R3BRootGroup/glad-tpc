/******************************************************************************
 * Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH *
 *         Copyright (C) 2018-2026 Members of R3B Collaboration               *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *              GNU Lesser General Public Licence (LGPL) version 3,           *
 *                     copied verbatim in the file "LICENSE".                 *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#pragma once

#include <boost/multi_array.hpp>

#include "TH2Poly.h"
#include "TObject.h"

#include <vector>

/**
 * Rectangular virtual-pad map for the GTPC projector.
 *
 * Coordinate/unit convention
 * --------------------------
 * All geometry quantities are in cm.
 * ---------------------------
 *
 * Pad ordering:
 *   padId = columnZ * numberOfRowsX + rowX
 *
 * Geometry convention
 * -------------------
 *   pitchX = padSizeX + gapX
 *   pitchZ = padSizeZ + gapZ
 *
 * The user supplies rows, columns, pad sizes and gaps explicitly.
 * The complete pitch grid is centred in the active X/Z face.
 *
 * Within each pitch cell the convention is:
 *
 *       |--------- physical pad ---------|--- gap ---|
 *       |<---------------- pitch ------------------->|
 *
 * i.e. the physical pad starts at the beginning of its pitch cell and the
 * configured gap follows the pad.  No half-gap shift is applied.
 *
 */

class R3BGTPCMap : public TObject
{

  public:
    R3BGTPCMap();
    ~R3BGTPCMap();

    typedef boost::multi_array<double, 3> multiarray;
    typedef multiarray::index index;

    /**
     * Configure the physical pad plane.
     *
     * sizeXcm/sizeZcm come from Active_region and are supplied by the
     * projector, not by the user macro.
     *
     * rowsX/columnsZ/pad sizes/gaps are the user-selectable readout settings.
     */
    Bool_t ConfigurePadGeometry(Double_t sizeXcm,
                                Double_t sizeZcm,
                                Int_t rowsX,
                                Int_t columnsZ,
                                Double_t padSizeXcm,
                                Double_t padSizeZcm,
                                Double_t gapXcm,
                                Double_t gapZcm);

    /** Build a TH2Poly representation for QA/drawing. */
    void GeneratePadPlane();

    /**
     * Find the pad corresponding to a point on the local readout face.
     *
     * Returns:
     *   >=0 : valid zero-based pad id
     *   -1  : outside the active face / invalid map
     *   -2  : inside active face but outside the instrumented pad matrix
     */
    Int_t FindPad(Double_t planeZcm, Double_t planeXcm) const;

    /** Convert a TH2Poly one-based bin index to zero-based pad id. */
    Int_t BinToPad(Int_t binval) const;

    /** Return {Zcenter, Xcenter} in cm relative to active-volume minima. */
    std::vector<Float_t> CalcPadCenter(Int_t padId) const;

    TH2Poly* GetPadPlane();
    const TH2Poly* GetPadPlane() const { return fPadPlane; }

    Int_t GetNumberOfRowsX() const { return fNumberOfRowsX; }
    Int_t GetNumberOfColumnsZ() const { return fNumberOfColumnsZ; }
    Int_t GetNumberOfPads() const { return fNumberOfRowsX * fNumberOfColumnsZ; }

    Double_t GetPadSizeXcm() const { return fPadSizeXcm; }
    Double_t GetPadSizeZcm() const { return fPadSizeZcm; }
    Double_t GetGapXcm() const { return fGapXcm; }
    Double_t GetGapZcm() const { return fGapZcm; }
    Double_t GetPitchXcm() const { return fPadSizeXcm + fGapXcm; }
    Double_t GetPitchZcm() const { return fPadSizeZcm + fGapZcm; }

    /** Readout plane of the pad map: +1 = local +Y face, -1 = local -Y face of the
     *  Active_region (set by R3BGTPCProjector::SetReadoutSide). */
    void SetReadoutSide(Int_t side) { fReadoutSide = side < 0 ? -1 : +1; }
    Int_t GetReadoutSide() const { return fReadoutSide; }

    Double_t GetSizeXcm() const { return fSizeXcm; }
    Double_t GetSizeZcm() const { return fSizeZcm; }

    /** Physical span: N pads plus N-1 internal gaps (no trailing edge gap). */
    Double_t GetGridSpanXcm() const;
    Double_t GetGridSpanZcm() const;

    /** Symmetric outer margin between active face and physical pad matrix. */
    Double_t GetGridOffsetXcm() const;
    Double_t GetGridOffsetZcm() const;

  private:
    Bool_t IsConfigured() const;

    multiarray fPadCoord;          //! derived from scalar geometry; rebuild with GeneratePadPlane()
    TH2Poly* fPadPlane{ nullptr }; //! QA/drawing object; never persisted

    Int_t fNumberOfRowsX{ 0 };
    Int_t fNumberOfColumnsZ{ 0 };

    Double_t fPadSizeXcm{ 0. };
    Double_t fPadSizeZcm{ 0. };
    Double_t fGapXcm{ 0. };
    Double_t fGapZcm{ 0. };

    Double_t fSizeXcm{ 0. };
    Double_t fSizeZcm{ 0. };

    Int_t fReadoutSide{ +1 }; // +1 / -1 = local +Y / -Y (version 4; +1, the projector default, in older files)

    ClassDefOverride(R3BGTPCMap, 4)
};
