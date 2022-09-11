#!/usr/bin/sh
#
# Rebuild flexemu and copy all needed libraries into the build folder.
#
# Before calling this script download_and_rebuild_libs.sh has to
# be executed before to create the Qt libraries and prepare the
# Visual Studio flexemu solution.
#
# syntax:
#    rebuild_flexemu_all_and_copy_libs.sh
# parameters:
#    none
#
#
# Convert an absolute path with Windows syntax into a path
# with MINGW syntax.
as_mingw_path() {
    expr='s:\\:/:g'
    path=`echo $1 | sed $expr`
    drive=`echo $path | sed -ne "s:\(.\).*:/\L\1:p"`
    path=`echo $path | sed "s/^..//"`
    echo ${drive}${path}
}

# Get the base path of the Microsoft Visual Studio installation.
# $1: Microsoft Visual Studio version ($vsversion)
get_vsbasedir() {
    if [ "$1" == "2019" ]; then
        echo "C:\Program Files (x86)\Microsoft Visual Studio"
    else
        echo "C:\Program Files\Microsoft Visual Studio"
    fi
}


# Read Microsoft Visual Studio Version and type from file
vsversion_file="vsversion.ini"
if [ ! -f $vsversion_file ]; then
    echo "**** File $vsversion_file does not exist."
    echo "**** First call download_and_rebuild_libs.sh to create libraries!"
    exit 1
fi

vsversion=`sed -ne "s/VS_VERSION\s*=\s*\(.*\)/\1/p" $vsversion_file`
vstype=`sed -ne "s/VS_TYPE\s*=\s*\(.*\)/\1/p" $vsversion_file`
if [ "x$vsversion" = "x" ] || [ "x$vstype" = "x" ]; then
    echo "**** Can't read Microsoft Visual Studio version or type."
    echo "**** Check file $vsversion_file. It should have a line"
    echo "**** VS_VERSION=... and VS_TYPE=..."
    exit 1
fi
vsbasedir=$( get_vsbasedir $vsversion )

VS160COMNTOOLS="$vsbasedir\\$vsversion\\$vstype\\Common7\\Tools\\"
if [ ! -d "$VS160COMNTOOLS" ]; then
    echo "**** Microsoft Visual Studio path not found:"
    echo "**** $VS160COMNTOOLS"
    echo "**** Check Your Microsoft Visual Studio installation."
    exit 1
fi
echo "Using Visual Studio $vsversion $vstype"

# Path of Qt build.
# Parse it from the *.props file.
propsfile="../../src/msvcQtPath.props"
if [ ! -f $propsfile ]; then
    echo "**** File ${propsfile} does not exist."
    echo "**** First call download_and_rebuild_libs.sh to create Qt libraries!"
    exit 1
fi
QTDIR=`sed -ne "s/<QTDIR>\(.*\)<\/QTDIR>/\1/p" $propsfile`
QTDIR=$( as_mingw_path $QTDIR)
qtversion=`echo $QTDIR | sed -e "s/.*\([56]\.[0-9]\+\.[0-9]\+\)$/\1/"`
qtmaversion=`echo $qtversion | sed -e "s/\([56]\).*/\1/"`

create_rebuild_script() {
    echo \@echo off >$2
    echo set DevEnvDir=$VS160COMNTOOLS\..\\IDE >>$2
    echo echo ... rebuild Release Win32 >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|Win32\" >>$2
    echo echo ... rebuild Release x64 >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|x64\" >>$2
    echo echo ... rebuild Debug Win32 >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|Win32\" >>$2
    echo echo ... rebuild Debug x64 >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|x64\" >>$2
}

temp=$QTDIR/x64/bin
if [ ! -d $temp ]; then
    echo "**** Qt libraries build directory does not exist:"
    echo "**** $temp"
    echo "**** First call download_and_rebuild_libs.sh to create Qt libraries!"
    exit 1
fi

echo "Using Qt V${qtversion}"

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
        if [ "$qtmaversion" = "5" ]; then
            qtlibs="Qt5Core Qt5Gui Qt5Widgets"
        else
            qtlibs="Qt6Core Qt6Gui Qt6Widgets"
        fi
        for file in $qtlibs
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
