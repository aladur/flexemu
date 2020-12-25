#!/usr/bin/sh
#
# syntax:
#    rebuild_flexemu_all_and_copy_libs.sh
# parameters:
#    none
#
if [ "x$VS160COMNTOOLS" = "x" ]; then
    # If not defined use the default path of the VS2019 community edition.
    # This path may evtl. has to be adapted.
    VS160COMNTOOLS="C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\Tools\\"
fi

# Path of Qt5 build.
# This path may evtl. has to be adapted.
QTDIR=/c/Qt/Qt5.15

create_rebuild_script() {
    echo \@echo off >$2
    echo set DevEnvDir=$VS160COMNTOOLS\..\\IDE >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|x64\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|x64\" >>$2
}

if [ ! -d $QTDIR/x64/bin ]; then
    echo "**** First call download_and_rebuild_libs.sh to create libraries!"
    exit 1
fi

cd ../..
# Create and execute a batch file to build all four configurations
create_rebuild_script flexemu.sln rebuild.bat
echo build flexemu...
$COMSPEC //C rebuild.bat

# check existence of result files and copy them to corresponding
# flexemu build directory

echo copy libraries...
if [ ! -d bin ]; then mkdir bin; fi
for platform in Win32 x64
do
    if [ ! -d bin/$platform ]; then mkdir bin/$platform; fi
    for configuration in Debug Release
    do
        postfix=
        if [ "x$configuration" = "xDebug" ]; then
            postfix=d
        fi
        targetdir=bin/$platform/$configuration
        if [ ! -d $targetdir ]; then
            mkdir $targetdir
        fi
        for file in Qt5Core Qt5Gui Qt5Widgets
        do
            cp -f ${QTDIR}/${platform}/bin/${file}${postfix}.dll $targetdir
        done
        if [ ! -d $targetdir/platforms ]; then
            mkdir $targetdir/platforms
        fi
        for file in qdirect2d qminimal qoffscreen qwindows
        do
            cp -f ${QTDIR}/${platform}/plugins/platforms/${file}${postfix}.dll $targetdir/platforms
        done
        if [ ! -d $targetdir/styles ]; then
            mkdir $targetdir/styles
        fi
        for file in qwindowsvistastyle
        do
            cp -f ${QTDIR}/${platform}/plugins/styles/${file}${postfix}.dll $targetdir/styles
        done
        cp -f src/flexemu.conf $targetdir
    done
done
cd build/windows
