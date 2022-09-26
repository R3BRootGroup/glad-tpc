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
# Create a library called "libR3BGTPCReconstruction" which includes the source files given in
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
${R3BGTPC_SOURCE_DIR}/gtpcdata
${R3BROOT_SOURCE_DIR}/field
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction/triplclust/src
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction/triplclust/src/hclust
${R3BGTPC_SOURCE_DIR}/gtpcreconstruction/triplclust/src/kdtree
${GENFIT2_INCLUDE_DIR}
)

include_directories( ${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${GENFIT2_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
triplclust/src/cluster.cxx 
triplclust/src/triplet.cxx 
#triplclust/src/main.cpp 
triplclust/src/dnn.cxx 
triplclust/src/hclust/fastcluster.cxx 
triplclust/src/kdtree/kdtree.cxx 
triplclust/src/pointcloud.cxx 
triplclust/src/output.cxx 
triplclust/src/option.cxx 
triplclust/src/util.cxx 
triplclust/src/graph.cxx
R3BGTPCTrackFinder.cxx
R3BGTPCFitter.cxx
R3BGTPCSpacePointMeasurement.cxx
)

# fill list of header files from list of source files
# by exchanging the file extension
CHANGE_FILE_EXTENSION(*.cxx *.h HEADERS "${SRCS}")

set(LINKDEF R3BGTPCReconstructionLinkDef.h)
set(LIBRARY_NAME R3BGTPCReconstruction)
set(DEPENDENCIES
    R3BBase R3BGTPCData ${GENFIT2_LIBRARY_DIR}/libgenfit2.so)

GENERATE_LIBRARY()
