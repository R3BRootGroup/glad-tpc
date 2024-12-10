##############################################################################
#   Copyright (C) 2018 GSI Helmholtzzentrum für Schwerionenforschung GmbH    #
#   Copyright (C) 2018-2025 Members of the R3B Collaboration                 #
#                                                                            #
#             This software is distributed under the terms of the            #
#                 GNU Lesser General Public Licence (GPL) version 3,         #
#                    copied verbatim in the file "LICENSE".                  #
#                                                                            #
# In applying this license GSI does not waive the privileges and immunities  #
# granted to it by virtue of its status as an Intergovernmental Organization #
# or submit itself to any jurisdiction.                                      #
##############################################################################

# Create a library called "libR3BGTPC" which includes the source files given in
# the array .

set(SRCS
    #Put here your sourcefiles
    R3BGTPC.cxx
    R3BGTPCProjector.cxx
    R3BGTPCLangevin.cxx
    R3BGTPCLaserGen.cxx
    R3BGTPCContFact.cxx
    R3BGTPCGeoPar.cxx
    R3BGTPCGasPar.cxx
    R3BGTPCElecPar.cxx
    #R3BGTPCHitPar.cxx
    R3BGTPCCalPar.cxx
    #R3BGTPCMappedPar.cxx
    R3BGTPCCal2Hit.cxx
    R3BGTPCMapped2Cal.cxx
    #R3BGTPCHit2Track.cxx
    #R3BGTPCCal2HitPar.cxx
    #R3BGTPCMapped2CalPar.cxx
)

# fill list of header files from list of source files
# by exchanging the file extension
change_file_extension(*.cxx *.h HEADERS "${SRCS}")

add_library_with_dictionary(
    LIBNAME
    R3BGTPC
    LINKDEF
    GTPCLinkDef.h
    HEADERS
    ${HEADERS}
    SRCS
    ${SRCS}
    INCLUDEDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDENCIES
    R3BGTPCData
    R3BGTPCMap)
