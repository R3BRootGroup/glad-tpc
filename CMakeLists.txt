
Set(R3BGTPC_SOURCE_DIR ${R3BROOT_SOURCE_DIR}/glad-tpc)

#glad-tpc specific
add_subdirectory (gtpcdata)
#if (WITH_UCESB)
#add_subdirectory (gtpcsource)
#endif (WITH_UCESB)
add_subdirectory (macros)
add_subdirectory (gtpc)

