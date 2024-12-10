#include "R3BGTPCMap.h"

R3BGTPCMap::R3BGTPCMap()
    : fPadCoord(boost::extents[5632][4][2])
{

    fPadCoord.resize(boost::extents[5632][4][2]);
    std::fill(fPadCoord.data(), fPadCoord.data() + fPadCoord.num_elements(), 0);
    fPadPlane = new TH2Poly();

    std::cout << " GLADTPC Map initialized " << std::endl;
    std::cout << " GLADTPC Pad Coordinates container initialized " << std::endl;
}

R3BGTPCMap::~R3BGTPCMap() {}

void R3BGTPCMap::GeneratePadPlane()
{

    Float_t pad_size = 2.0;      // mm
    Float_t pad_spacing = 0.001; // mm

    Float_t ZOffset = 0.0; // 272.7;
    Float_t XOffset = 0.0; // 5.8;

    Int_t padCnt = 0;

    // x - y (Z - X in GLAD convention)
    for (auto icol = 0; icol < 128; ++icol)
        for (auto irow = 0; irow < 44; ++irow)
        {
            fPadCoord[padCnt][0][0] = pad_size * (Float_t)icol + ZOffset;
            fPadCoord[padCnt][0][1] = pad_size * (Float_t)irow + XOffset;
            fPadCoord[padCnt][1][0] = pad_size * (Float_t)icol + ZOffset;
            fPadCoord[padCnt][1][1] = pad_size * (Float_t)irow + pad_size + XOffset;
            fPadCoord[padCnt][2][0] = pad_size * (Float_t)icol + pad_size + ZOffset;
            fPadCoord[padCnt][2][1] = pad_size * (Float_t)irow + pad_size + XOffset;
            fPadCoord[padCnt][3][0] = pad_size * (Float_t)icol + pad_size + ZOffset;
            fPadCoord[padCnt][3][1] = pad_size * (Float_t)irow + XOffset;
            ++padCnt;
        }

    for (auto ipad = 0; ipad < 5632; ++ipad)
    {
        Double_t px[] = { fPadCoord[ipad][0][0],
                          fPadCoord[ipad][1][0],
                          fPadCoord[ipad][2][0],
                          fPadCoord[ipad][3][0],
                          fPadCoord[ipad][0][0] };
        Double_t py[] = { fPadCoord[ipad][0][1],
                          fPadCoord[ipad][1][1],
                          fPadCoord[ipad][2][1],
                          fPadCoord[ipad][3][1],
                          fPadCoord[ipad][0][1] };
        Int_t bin = fPadPlane->AddBin(5, px, py);
    }
}

std::vector<Float_t> R3BGTPCMap::CalcPadCenter(Int_t PadRef)
{

    std::vector<Float_t> PadCenter = { -9999, -9999 };
    PadCenter.reserve(2);

    if (fPadPlane == 0)
    {

        // std::cout << " R3BGTPCMap::CalcPadCenter Error : Pad plane not found" <<
        // std::endl;
        return PadCenter;
    }

    if (PadRef >= 0 && PadRef < fPadCoord.shape()[0])
    { // Boost multi_array crashes with a negative index

        Float_t x = (fPadCoord[PadRef][0][0] + fPadCoord[PadRef][3][0]) / 2.0;
        PadCenter[0] = x;
        Float_t y = (fPadCoord[PadRef][0][1] + fPadCoord[PadRef][1][1]) / 2.0;
        PadCenter[1] = y;
        return PadCenter;
    }
    else
    {
        std::cout << PadRef << " ; " << fPadCoord.shape()[0] << '\n';
        std::cout << " R3BGTPCMap::CalcPadCenter Error : Pad not found" << std::endl;
        return PadCenter;
    }
}

TH2Poly* R3BGTPCMap::GetPadPlane()
{

    if (fPadPlane == 0)
    {

        std::cout << " R3BGTPCMap::GetAtTPCPlane Error : Pad plane has not been "
                     "generated - Exiting... "
                  << std::endl;

        return NULL;
    }

    fPadPlane->SetName("R3BGTPC_Plane");
    fPadPlane->SetTitle("R3BGTPC_Plane");
    fPadPlane->ChangePartition(500, 500);

    return fPadPlane;
}

Int_t R3BGTPCMap::BinToPad(Int_t binval) { return 0; }

ClassImp(R3BGTPCMap)
