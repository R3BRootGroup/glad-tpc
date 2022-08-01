##############################################################################
#   Copyright (C) 2019 GSI Helmholtzzentrum für Schwerionenforschung GmbH    #
#   Copyright (C) 2019 Members of R3B Collaboration                          #
#                                                                            #
#             This software is distributed under the terms of the            #
#                 GNU General Public Licence (GPL) version 3,                #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################
# Create a library called "libR3BGTPC" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(SYSTEM_INCLUDE_DIRECTORIES
${SYSTEM_INCLUDE_DIRECTORIES}
${BASE_INCLUDE_DIRECTORIES}
)

set(INCLUDE_DIRECTORIES
#put here all directories where header files are located
${R3BROOT_SOURCE_DIR}/r3bbase
${R3BROOT_SOURCE_DIR}/r3bdata
${R3BGTPC_SOURCE_DIR}/gtpc
${R3BGTPC_SOURCE_DIR}/gtpcmap
${R3BGTPC_SOURCE_DIR}/gtpcdata
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction/triplclust/src
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction/
${R3BROOT_SOURCE_DIR}/field
${Boost_INCLUDE_DIRS}
)

include_directories( ${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
#Put here your sourcefiles
R3BGTPC.cxx
R3BGTPCProjector.cxx
R3BGTPCLangevin.cxx
R3BGTPCLangevinTest.cxx
R3BGTPCContFact.cxx
R3BGTPCGeoPar.cxx
R3BGTPCGasPar.cxx
R3BGTPCElecPar.cxx
#R3BGTPCHitPar.cxx
R3BGTPCCalPar.cxx
#R3BGTPCMappedPar.cxx
R3BGTPCCal2Hit.cxx
R3BGTPCMapped2Cal.cxx
R3BGTPCHit2Track.cxx
#R3BGTPCCal2HitPar.cxx
#R3BGTPCMapped2CalPar.cxx
)

# fill list of header files from list of source files
# by exchanging the file extension
CHANGE_FILE_EXTENSION(*.cxx *.h HEADERS "${SRCS}")

set(LINKDEF GTPCLinkDef.h)
set(LIBRARY_NAME R3BGTPC)
set(DEPENDENCIES
    R3BBase R3BGTPCData R3BGTPCMap R3BGTPCReconstruction)

GENERATE_LIBRARY()
