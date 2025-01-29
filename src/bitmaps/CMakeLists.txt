install(FILES
    16x16/flexemu.png
    16x16/flexemu-flexplorer.png
    16x16/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/16x16/apps
)

install(FILES
    32x32/flexemu.png
    32x32/flexemu-flexplorer.png
    32x32/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/32x32/apps
)

install(FILES
    48x48/flexemu.png
    48x48/flexemu-flexplorer.png
    48x48/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/48x48/apps
)

install(FILES
    64x64/flexemu.png
    64x64/flexemu-flexplorer.png
    64x64/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/64x64/apps
)

install(FILES
    128x128/flexemu.png
    128x128/flexemu-flexplorer.png
    128x128/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/128x128/apps
)

install(FILES
    256x256/flexemu.png
    256x256/flexemu-flexplorer.png
    256x256/flexemu-fsetup.png
    DESTINATION share/icons/hicolor/256x256/apps
)

install(FILES
    scalable/flexemu.svg
    scalable/flexemu-flexplorer.svg
    scalable/flexemu-fsetup.svg
    DESTINATION share/icons/hicolor/scalable/apps
)

# Icon for FLEX disk image mimetype.
# See also file src/flexemu.xml referencing the icon name.
# Store as *.png file, this is the default format.
install(FILES
    48x48/flexemu-flexplorer.png
    DESTINATION share/icons/hicolor/48x48/mimetypes
    RENAME application-x-flex-diskimage.png
)

# Store as *.svg file, this is optional.
install(FILES
    scalable/flexemu-flexplorer.svg
    DESTINATION share/icons/hicolor/scalable/mimetypes
    RENAME application-x-flex-diskimage.svg
)
