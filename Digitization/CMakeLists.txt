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

# Create a library called "libR3BGTPCDigitization" with the VMM3a digitizer
# (R3BGTPCProjPoint -> R3BGTPCVMMDigi) and the MC-truth match object.

set(SRCS
    R3BGTPCVMMDigi.cxx
    R3BGTPCMatch.cxx
    R3BGTPCVMMDigitizer.cxx)

# fill list of header files from list of source files
# by exchanging the file extension
change_file_extension(*.cxx *.h HEADERS "${SRCS}")

add_library_with_dictionary(
    LIBNAME
    R3BGTPCDigitization
    LINKDEF
    R3BGTPCDigitizationLinkDef.h
    HEADERS
    ${HEADERS}
    SRCS
    ${SRCS}
    INCLUDEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDENCIES
    R3BGTPCData
    R3BGTPCMap
    R3BGTPC)

target_compile_options(R3BGTPCDigitization PRIVATE -Wno-overloaded-virtual)
