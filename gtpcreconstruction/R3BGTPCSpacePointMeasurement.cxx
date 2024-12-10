#include "R3BGTPCSpacePointMeasurement.h"

#include "R3BGTPCHitClusterData.h"

#include <Math/Point3D.h>
#include <TMatrixDSymfwd.h>
#include <TMatrixDfwd.h>
#include <TMatrixT.h>
#include <TMatrixTSym.h>
#include <TVectorDfwd.h>
#include <TVectorT.h>
#include <TrackCandHit.h>

#include <SpacepointMeasurement.h>

namespace genfit
{
    class AbsMeasurement;
} // namespace genfit

ClassImp(genfit::R3BGTPCSpacepointMeasurement)

    namespace genfit
{

    R3BGTPCSpacepointMeasurement::R3BGTPCSpacepointMeasurement()
        : SpacepointMeasurement()
    {
    }

    R3BGTPCSpacepointMeasurement::R3BGTPCSpacepointMeasurement(const R3BGTPCHitClusterData* detHit,
                                                               const TrackCandHit* hit)
        : SpacepointMeasurement()
        , fCharge(detHit->GetEnergy())
    {

        TMatrixD mat = detHit->GetCovMatrix();

        // std::cout<<detHit->GetX()<<" "<<detHit->GetY()<<"
        // "<<detHit->GetZ()<<"\n";

        rawHitCoords_(0) = detHit->GetX();
        rawHitCoords_(1) = detHit->GetY();
        rawHitCoords_(2) = detHit->GetZ();

        TMatrixDSym cov(3);

        /*cov(0,0) = detHit -> GetDx();
        cov(1,1) = detHit -> GetDy();
        cov(2,2) = detHit -> GetDz();*/  //TODO: Compute position variance

        cov(0, 1) = 0.0;
        cov(1, 2) = 0.0;
        cov(2, 0) = 0.0;

        // Forced covariance matrix to be constant. Need to study later.
        cov(0, 0) = 0.1;
        cov(1, 1) = 0.4;
        cov(2, 2) = 0.1;

        rawHitCov_ = cov;
        detId_ = hit->getDetId();
        hitId_ = hit->getHitId();

        // std::cout<<" AtSpacepointMeasurement::AtSpacepointMeasurement "<<"\n";
        // std::cout<<rawHitCoords_(0)<<"	"<<rawHitCoords_(1)<<"
        // "<<rawHitCoords_(2)<<"	"<<fCharge<<"
        // "<<detId_<<"  "<<hitId_<<"\n";

        this->initG();
    }

    AbsMeasurement* R3BGTPCSpacepointMeasurement::clone() const { return new R3BGTPCSpacepointMeasurement(*this); }

    Double_t R3BGTPCSpacepointMeasurement::GetCharge() { return fCharge; }

} /* End of namespace genfit */
