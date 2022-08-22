cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(r3bgtpcfitter)

SET(SIMPATH $ENV{SIMPATH})
SET(FAIRROOTPATH $ENV{FAIRROOTPATH})
SET(R3BROOTPATH "/mnt/simulations/attpcroot/fair_install_2020/R3BRoot/")
    
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

message(${CMAKE_MODULE_PATH})

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})


find_package(ROOT REQUIRED)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

#---Locate other external packages
find_package(Boost 1.67 COMPONENTS system REQUIRED)
find_package(GENFIT2)


include_directories( INCLUDE_DIRECTORIES
                     ${ROOT_INCLUDE_DIR}
                     ${Boost_INCLUDE_DIRS}
                     ${BASE_INCLUDE_DIRECTORIES}
                     ${R3BROOTPATH}/glad-tpc/gtpcdata
		     ${R3BROOTPATH}/glad-tpc/gtpcreconstruction
                     ${FAIRROOTPATH}/include
                     ${CMAKE_SOURCE_DIR}
		     ${GENFIT2_INCLUDE_DIR}
		     )

link_directories(LINK_DIRECTORIES
		  ${FAIRROOT_LIBRARY_DIR}
                  ${FAIRROOTPATH}/lib
                  ${R3BROOTPATH}/build/lib
                  ${GENFIT2_LIBRARY_DIR}
		  ${Boost_LOG_LIBRARY}
		  ${Boost_LOG_SETUP_LIBRARY}
		  ${Boost_THREAD_LIBRARY}
		  ${Boost_DAtE_TIME_LIBRARY}
		  ${Boost_FILESYSTEM_LIBRARY}
		  ${Boost_SYSTEM_LIBRARY}                  
                  )

message(STATUS ${R3BROOTPATH})

add_library(fitter SHARED fitter.cxx)
target_link_libraries(fitter ${ROOT_LIBRARIES} R3BGTPCReconstruction   ${GENFIT2_LIBRARY_DIR}/libgenfit2.so)

add_executable(fitterExe fitter.cxx)

target_link_libraries (fitterExe fitter
${ROOT_LIBRARIES}
${GENFIT2_LIBRARY_DIR}/libgenfit2.so
${Boost_LOG_LIBRARY}
${Boost_LOG_SETUP_LIBRARY}
${Boost_THREAD_LIBRARY}
${Boost_DAtE_TIME_LIBRARY}
${Boost_FILESYSTEM_LIBRARY}
${Boost_SYSTEM_LIBRARY}
R3BGTPCReconstruction
EG
Core
Gpad
Graf
Graf3d
Gui
Hist
MathCore
Matrix
Minuit
Net
Physics
Postscript
Rint
RIO
Thread
Tree
TreePlayer
FairTools
Geom
Spectrum
Base
)
