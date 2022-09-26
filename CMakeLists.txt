# The name of our project is “R3BGTPC”.CMakeLists files in this project can
# refer to the R3BROOT root source directory of the project as ${R3BROOT_SOURCE_DIR},
# the SOFIA root source directory as ${R3BGTPC_SOURCE_DIR}(for the gtpc specifics) or
# as ${CMAKE_SOURCE_DIR} and to the root binary directory of the project as
# ${R3BGTPC_BINARY_DIR} or ${CMAKE_BINARY_DIR}.

# Recurse into the given subdirectories.This does not actually cause
# another cmake executable to run.The same process will walk through
# the project's entire directory structure.

Set(R3BGTPC_SOURCE_DIR ${R3BROOT_SOURCE_DIR}/glad-tpc)
message(STATUS "${BBlue}GTPC project was FOUND${CR} ")
message("    - ${Cyan}GTPC_DIR${CR} = ${BGreen}${R3BGTPC_SOURCE_DIR}${CR}")

find_package(GENFIT2)

#glad-tpc specific
add_subdirectory (gtpcdata)
#if (WITH_UCESB)
#add_subdirectory (gtpcsource)
#endif (WITH_UCESB)
add_subdirectory (macros)
add_subdirectory (gtpc)
add_subdirectory (gtpceve)
add_subdirectory (gtpcreconstruction)
add_subdirectory (gtpcmap)
