set(install_FILES
    2x384flx.png
    2x384k.png
    2x96k.png
    e2scn.png
    e2v5m.png
    e2v7m.png
    pat09_cursorpad.png
    pat09_keyboard.png
    pat09_key_down_arrow.png
    pat09_key_left_arrow.png
    pat09_key_left_limit.png
    pat09_key_mode.png
    pat09_key_right_arrow.png
    pat09_key_right_limit.png
    pat09_key_thick_left_arrow.png
    pat09_key_thick_lower_right_arrow.png
    pat09_key_thick_right_arrow.png
    pat09_key_thick_upper_left_arrow.png
    pat09_key_up_arrow.png
    pat09_numpad.png
)

install(FILES ${install_FILES} DESTINATION ${FLEXEMU_DATADIR}/images)
