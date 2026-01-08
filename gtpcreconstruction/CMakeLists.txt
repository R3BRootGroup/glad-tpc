##############################################################################
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
##############################################################################

# Create a library called "libR3BGTPCReconstruction" which includes the source
# files given in the array.

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${VMCWORKDIR}/glad-tpc/fitter/cmake/modules"
)

find_package(GENFIT2 QUIET)

set(SRCS
    triplclust/src/cluster.cxx
    triplclust/src/triplet.cxx
    triplclust/src/dnn.cxx
    triplclust/src/hclust/fastcluster.cxx
    triplclust/src/kdtree/kdtree.cxx
    triplclust/src/pointcloud.cxx
    triplclust/src/output.cxx
    triplclust/src/option.cxx
    triplclust/src/util.cxx
    triplclust/src/graph.cxx
    R3BGTPCTrackFinder.cxx
)

if(GENFIT2_FOUND)

    list(APPEND SRCS
        R3BGTPCFitter.cxx
        R3BGTPCSpacePointMeasurement.cxx
    )

    list(APPEND EXTRA_INCLUDE_DIRS
        ${GENFIT2_INCLUDE_DIR}
    )
endif()

# Fill list of header files from list of source files
change_file_extension(*.cxx *.h HEADERS "${SRCS}")

add_library_with_dictionary(
    LIBNAME
        R3BGTPCReconstruction
    LINKDEF
        R3BGTPCReconstructionLinkDef.h
    HEADERS
        ${HEADERS}
    SRCS
        ${SRCS}
    INCLUDEDIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/triplclust/src
        ${EXTRA_INCLUDE_DIRS}
    DEPENDENCIES
        R3BGTPCData
)