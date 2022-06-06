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
)

include_directories( ${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
triplclust/src/cluster.cpp 
triplclust/src/triplet.cpp 
triplclust/src/main.cpp 
triplclust/src/dnn.cpp 
triplclust/src/hclust/fastcluster.cpp 
triplclust/src/kdtree/kdtree.cpp 
triplclust/src/pointcloud.cpp 
triplclust/src/output.cpp 
triplclust/src/option.cpp 
triplclust/src/util.cpp 
triplclust/src/graph.cpp
R3BGTPCTrackFinder.cxx
)

# fill list of header files from list of source files
# by exchanging the file extension
CHANGE_FILE_EXTENSION(*.cxx *.h HEADERS "${SRCS}")

#set(LINKDEF R3BGTPCReconstructionLinkDef.h)
set(LIBRARY_NAME R3BGTPCReconstruction)
set(DEPENDENCIES
    R3BBase R3BGTPCData)

GENERATE_LIBRARY()
