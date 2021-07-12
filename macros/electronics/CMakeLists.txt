# CMakeLists.txt for event package. It creates a library with dictionary and a main program
#Minimum cmake version
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
#project name 
project(AGET_Electronics)

# Check for needed environment variables
IF(NOT DEFINED ENV{FAIRROOTPATH})
  MESSAGE(FATAL_ERROR "You did not define the environment variable FAIRROOTPATH which is needed to find FairRoot. Please set this variable and execute cmake again.")
ENDIF(NOT DEFINED ENV{FAIRROOTPATH})

IF(NOT DEFINED ENV{SIMPATH})
   MESSAGE(FATAL_ERROR "You did not define the environment variable SIMPATH which is nedded to find the external packages. Please set this variable and execute cmake again.")
ENDIF(NOT DEFINED ENV{SIMPATH})
#Fairinstall
SET(SIMPATH $ENV{SIMPATH})
#Fairroot
SET(FAIRROOTPATH $ENV{FAIRROOTPATH})
#glad-tpc folder
SET(GTPCROOTPATH $ENV{VMCWORKDIR}/glad-tpc)
#R3BGTPC library
SET(FAIRLIBDIR $ENV{FAIRLIBDIR})

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS RIO)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

#---Locate other external packages


include_directories( ${ROOT_INCLUDE_DIR}
                     ${BASE_INCLUDE_DIRECTORIES}
                     ${GTPCROOTPATH}
                     ${FAIRROOTPATH}/include
                     ${CMAKE_SOURCE_DIR})

link_directories( ${FAIRROOT_LIBRARY_DIR}
                  ${FAIRROOTPATH}/lib
                  ${FAIRLIBDIR}
                  )

#Add exectubale target which will be built
message(STATUS ${GTPCROOTPATH})

add_library(Electronics SHARED Electronics_MT.C)
target_link_libraries(Electronics ${ROOT_LIBRARIES} R3BGTPC R3BGTPCData FairTools Base )

add_executable(AGetElectronics Electronics_MT.C)
target_link_libraries (AGetElectronics Electronics
 ${ROOT_LIBRARIES}
Hist
R3BGTPC
R3BGTPCData
Rint
Tree
FairTools
Base)
