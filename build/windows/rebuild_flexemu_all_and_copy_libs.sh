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

VSMSBUILDDIR="$vsbasedir\\$vsversion\\$vstype\\MSBuild\\Current\\Bin"
if [ ! -d "$VSMSBUILDDIR" ]; then
    echo "**** Microsoft Visual Studio path not found:"
    echo "**** $VSMSBUILDDIR"
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
QTDIR=$( as_mingw_path "$QTDIR")
qtversion=`echo $QTDIR | sed -e "s/.*\([56]\.[0-9]\+\.[0-9]\+\)$/\1/"`
qtmaversion=`echo $qtversion | sed -e "s/\([56]\).*/\1/"`

temp=$QTDIR/x64/bin
if [ ! -d $temp ]; then
    echo "**** Qt libraries build directory does not exist:"
    echo "**** $temp"
    echo "**** First call download_and_rebuild_libs.sh to create Qt libraries!"
    exit 1
fi

echo "Using Qt V${qtversion}"

cd ../..
vsmsbuilddir=$( as_mingw_path "$VSMSBUILDDIR")

# Rebuild flexemu solution for all platforms and configurations.
# Check existence of result files and copy them to corresponding
# flexemu build directory.

if [ ! -d bin ]; then mkdir bin; fi
if [ ! -d bin/Qt${qtversion} ]; then mkdir bin/Qt${qtversion}; fi
if [ "$qtmaversion" = "5" ]; then
    platforms="Win32 x64"
else
    platforms="x64"
fi
for platform in $platforms
do
    if [ ! -d bin/$platform ]; then mkdir bin/$platform; fi
    for configuration in Debug Release
    do
        logfile=Build-flexemu-${platform}-${configuration}.log
        echo ==== Rebuild flexemu solution $configuration $platform ...
        echo For details see log-file "$logfile".
        "$vsmsbuilddir/MSBuild.exe" flexemu.sln -t:Rebuild -m -v:quiet -clp:Summary -fl -flp:Verbosity=minimal\;LogFile=$logfile -property:Configuration=${configuration}\;Platform=${platform}
        if [[ $? == 0 ]]; then
            echo ==== Copy libraries ...
            postfix=
            if [ "x$configuration" = "xDebug" ]; then
                postfix=d
            fi
            targetdir=bin/Qt${qtversion}/$platform
            if [ ! -d $targetdir ]; then
                mkdir $targetdir
            fi
            targetdir=${targetdir}/$configuration
            if [ ! -d $targetdir ]; then
                mkdir $targetdir
            fi
            if [ "$qtmaversion" = "5" ]; then
                qtlibs="Qt5Core Qt5Gui Qt5Widgets Qt5PrintSupport"
            else
                qtlibs="Qt6Core Qt6Gui Qt6Widgets Qt6PrintSupport"
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
            if [ "$qtmaversion" = "5" ]; then
                if [ ! -d $targetdir/printsupport ]; then
                    mkdir $targetdir/printsupport
                fi
                for file in windowsprintersupport
                do
                    cp -f ${QTDIR}/${platform}/plugins/printsupport/${file}${postfix}.dll $targetdir/printsupport
                done
            fi
            cp -f src/flexemu.conf $targetdir
        fi
    done
done
cd build/windows
