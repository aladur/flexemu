set(install_FILES
    btx.dsk
    cedric.dsk
    colors.dsk
    diag6809.dsk
    dynadocu.dsk
    games.dsk
    just.dsk
    laycad.dsk
    layout.dsk
    pictures.dsk
    source.dsk
    system54.dsk
    system.dsk
    system.mdcr
    test.dsk
    tsc_man.dsk
)

install(FILES ${install_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/flexemu)
