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

#include "R3BGTPCContFact.h"
#include "FairLogger.h"
#include "FairRuntimeDb.h"
#include "R3BGTPCElecPar.h"
#include "R3BGTPCGasPar.h"
#include "R3BGTPCGeoPar.h"

static R3BGTPCContFact gR3BGTPCContFact;

R3BGTPCContFact::R3BGTPCContFact()
{
    // Constructor (called when the library is loaded)

    fName = "R3BGTPCContFact";
    fTitle = "Factory for GTPC parameter containers in libR3BGTPC";
    setAllContainers();
    FairRuntimeDb::instance()->addContFactory(this);
}

void R3BGTPCContFact::setAllContainers()
{
    // Creates the Container objects with all accepted contexts and adds them
    // to the list of containers for the GTPC library.

    FairContainer* p1 = new FairContainer("GTPCGeoPar", "GTPC Geometry Parameters", "GTPCGeoParContext");
    p1->addContext("GTPCGeoParContext");
    containers->Add(p1);

    FairContainer* p2 = new FairContainer("GTPCGasPar", "GTPC Gas Parameters", "GTPCGasParContext");
    p2->addContext("GTPCGasParContext");
    containers->Add(p2);

    FairContainer* p3 = new FairContainer("GTPCElecPar", "GTPC Electronic Parameters", "GTPCElecParContext");
    p3->addContext("GTPCElecParContext");
    containers->Add(p3);
}

FairParSet* R3BGTPCContFact::createContainer(FairContainer* c)
{
    /** Calls the constructor of the corresponding parameter container.
     * For an actual context, which is not an empty string and not the default
     * context of this container, the name is concatenated with the context.
     */

    const char* name = c->GetName();
    LOG(info) << "R3BGTPCContFact: Create container name: " << name;
    FairParSet* p = 0;

    if (strcmp(name, "GTPCGeoPar") == 0)
    {
        p = new R3BGTPCGeoPar(c->getConcatName().Data(), c->GetTitle(), c->getContext());
    }

    if (strcmp(name, "GTPCGasPar") == 0)
    {
        p = new R3BGTPCGasPar(c->getConcatName().Data(), c->GetTitle(), c->getContext());
    }

    if (strcmp(name, "GTPCElecPar") == 0)
    {
        p = new R3BGTPCElecPar(c->getConcatName().Data(), c->GetTitle(), c->getContext());
    }

    return p;
}

ClassImp(R3BGTPCContFact)
