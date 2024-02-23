/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert & Johannes Rauch
   This file is part of GENFIT.
   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/

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
