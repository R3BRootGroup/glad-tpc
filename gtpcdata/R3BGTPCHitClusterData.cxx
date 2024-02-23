#include "R3BGTPCHitClusterData.h"

#include <Rtypes.h>

ClassImp(R3BGTPCHitClusterData);

R3BGTPCHitClusterData::R3BGTPCHitClusterData()
    : R3BGTPCHitData(-10000, -10000, -10000, 0.0, 0.0)
{
    fCovMatrix.ResizeTo(3, 3);
    for (Int_t iElem = 0; iElem < 9; iElem++)
        fCovMatrix(iElem / 3, iElem % 3) = 0;
}
