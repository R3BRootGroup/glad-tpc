
/*##############################################################################
# Copyright (C) 2018-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH #
#   Copyright (C) 2018-2026 Members of the R3B Collaboration                 #
#                                                                            #
#            This software is distributed under the terms of the             #
#              GNU Lesser General Public Licence (GPL) version 3,            #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################*/

#ifndef R3BGTPCSPACEPOINTMEASUREMENT
#define R3BGTPCSPACEPOINTMEASUREMENT

#include <Rtypes.h>

#include "SpacepointMeasurement.h"

class R3BGTPCHitClusterData;
class TBuffer;
class TClass;
class TMemberInspector;

namespace genfit
{
    class AbsMeasurement;
    class TrackCandHit;

    class R3BGTPCSpacepointMeasurement : public SpacepointMeasurement
    {

      public:
        R3BGTPCSpacepointMeasurement();
        R3BGTPCSpacepointMeasurement(const R3BGTPCHitClusterData* detHit, const TrackCandHit* hit);

        virtual AbsMeasurement* clone() const;

        Double_t GetCharge();

      private:
        Double_t fCharge{};

        ClassDef(R3BGTPCSpacepointMeasurement, 1)
    };

} /* End of namespace genfit */

#endif
