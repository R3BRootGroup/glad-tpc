# Create a library called "libR3BGTPCMAP" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")

Set(SYSTEM_INCLUDE_DIRECTORIES
${SYSTEM_INCLUDE_DIRECTORIES}
${BASE_INCLUDE_DIRECTORIES}
)

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${Boost_INCLUDE_DIRS}
${R3BGTPC_SOURCE_DIR}/gtpcmap
)

include_directories( ${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
R3BGTPCMap.cxx
)

# fill list of header files from list of source files
# by exchanging the file extension
CHANGE_FILE_EXTENSION(*.cxx *.h HEADERS "${SRCS}")

set(LINKDEF  R3BGTPCMapLinkDef.h)
set(LIBRARY_NAME R3BGTPCMap)
set(DEPENDENCIES
    Hist Core m)

GENERATE_LIBRARY()

