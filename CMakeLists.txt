
Set(R3BGTPC_SOURCE_DIR ${R3BROOT_SOURCE_DIR}/glad-tpc)

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
