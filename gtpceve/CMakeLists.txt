##############################################################################
#   Copyright (C) 2019-2026 GSI Helmholtzzentrum für Schwerionenforschung GmbH    #
#   Copyright (C) 2019-2026 Members of R3B Collaboration                          #
#                                                                            #
#             This software is distributed under the terms of the            #
#                 GNU Lesser General Public Licence (LGPL) version 3,                #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################
# Create a library called "libR3BGTPC" which includes the source files given in
# the array .

set(SRCS R3BGTPCEventManager.cxx R3BGTPCEventDrawTask.cxx)

# fill list of header files from list of source files
# by exchanging the file extension
change_file_extension(*.cxx *.h HEADERS "${SRCS}")

add_library_with_dictionary(
    LIBNAME
    R3BGTPCEve
    LINKDEF
    GTPCEveLinkDef.h
    HEADERS
    ${HEADERS}
    SRCS
    ${SRCS}
    INCLUDEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDENCIES
    R3BGTPCData
    R3BGTPCMap
    Eve Gui Ged RGL Graf3d)
