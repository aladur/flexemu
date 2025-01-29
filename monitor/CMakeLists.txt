set(install_FILES
    coltab.hex
    mon24.s19
    mon24z.s19
    mon53.s19
    mon54.s19
    monu54-6.s19
    neumon54.hex
)

install(FILES ${install_FILES} DESTINATION ${FLEXEMU_DATADIR})
