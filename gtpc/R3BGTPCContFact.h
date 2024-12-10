/******************************************************************************
 *   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    *
 *   Copyright (C) 2018-2025 Members of R3B Collaboration                     *
 *                                                                            *
 *             This software is distributed under the terms of the            *
 *                 GNU Lesser General Public Licence (LGPL) version 3,        *
 *                    copied verbatim in the file "LICENSE".                  *
 *                                                                            *
 * In applying this license GSI does not waive the privileges and immunities  *
 * granted to it by virtue of its status as an Intergovernmental Organization *
 * or submit itself to any jurisdiction.                                      *
 ******************************************************************************/

#pragma once

#include "FairContFact.h"

class FairContainer;

/**
 * GTPC parameter container factory
 */

class R3BGTPCContFact : public FairContFact
{
  private:
    void setAllContainers();

  public:
    R3BGTPCContFact();
    //~R3BGTPCContFact() {}
    FairParSet* createContainer(FairContainer*) override;
    ClassDefOverride(R3BGTPCContFact, 0)
};
