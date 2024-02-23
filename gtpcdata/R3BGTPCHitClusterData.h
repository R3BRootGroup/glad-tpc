#ifndef R3BGTPCHITCLUSTERDATA_H
#define R3BGTPCHITCLUSTERDATA_H

#include "R3BGTPCHitData.h"

#include <Rtypes.h>
#include <TMatrixDfwd.h>
#include <TMatrixT.h>

class R3BGTPCHitClusterData : public R3BGTPCHitData
{

  protected:
    TMatrixD fCovMatrix;

    Double_t fLength = -999;
    Int_t fClusterID = -1;

  public:
    R3BGTPCHitClusterData();
    R3BGTPCHitClusterData(const R3BGTPCHitClusterData& cluster) = default;
    virtual ~R3BGTPCHitClusterData() = default;

    void SetCovMatrix(const TMatrixD& matrix) { fCovMatrix = matrix; }
    void SetLength(Double_t length) { fLength = length; }
    void SetClusterID(Int_t id) { fClusterID = id; }

    Double_t GetLength() const { return fLength; }
    const TMatrixD& GetCovMatrix() const { return fCovMatrix; }
    Int_t GetClusterID() const { return fClusterID; }

    ClassDef(R3BGTPCHitClusterData, 1);
};

#endif
