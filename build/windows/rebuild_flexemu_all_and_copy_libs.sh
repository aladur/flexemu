#!/usr/bin/sh
#
# Rebuild flexemu and copy all needed libraries and files into the build folder.
#
# Before calling this script download_and_rebuild_libs.sh has to
# be executed before to create the Qt libraries and prepare the
# Visual Studio flexemu solution.
#
# syntax:
#    rebuild_flexemu_all_and_copy_libs.sh [-c]
# parameters:
#    -c:            Use cmake to build flexemu.
#
#

function usage() {
    echo "Rebuild flexemu and copy all needed libraries and files into the build folder."
    echo ""
    echo "Syntax:"
    echo "   rebuild_flexemu_all_and_copy_libs.sh [-c]"
    echo "Options:"
    echo "   -c:             Use cmake to build flexemu (default: use flexemu.sln)"
}

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

# Convert an absolute path with MINGW syntax into a path
# with Windows syntax.
as_windows_path() {
    path=`echo $1 | sed "s/^.//"`
    expr='s:/:\\:g'
    path=`echo $path | sed $expr`
    drive=`echo $path | sed "s:\(^.\).*:\U\1:"`
    path=`echo $path | sed "s/^.//"`
    echo $drive:$path
}

# Convert a MINGW path relative to the current directory
# into an absolute path with Windows syntax.
as_absolute_windows_path() {
    absdir=`pwd`
    absdir="$absdir/$1"
    absdir=$( as_windows_path $absdir )
    echo $absdir
}

# check string against a list of supported strings.
# $1: The string to check.
# $2: Space separated list of supported strings.
check_value() {
    if [ "x$1" == "x" ]; then
        return 0; # no value is valid
    fi
    for value in $2
    do
        if [ "$value" == "$1" ]; then
            return 0
        fi
    done
    return 1
}

# Function to copy the Qt libraries.
# $1: Qt directory (source directory for copy)
# $2: qtversion (major.minor.patch)
# $3: platform (Win32 or x64)
# $4: configuration (Debug or Release)
# $5: path where flexemu project executables are created, if empty ignore
copy_files() {
    echo ==== Copy files ...
    qtmaver=`echo $2 | sed -e "s/\([56]\).*/\1/"`
    qtmiver=`echo $qtversion | sed -e "s/[0-9]\+.\([0-9]\+\).*/\1/"`
    postfix=
    if [ "x$4" = "xDebug" ]; then
        postfix=d
    fi
    targetdir=bin/Qt$2/$3
    if [ ! -d $targetdir ]; then
        mkdir $targetdir
    fi
    targetdir=${targetdir}/$4
    if [ ! -d $targetdir ]; then
        mkdir $targetdir
    fi
    if [ "$qtmaver" = "5" ]; then
        qtlibs="Qt5Core Qt5Gui Qt5Widgets Qt5PrintSupport"
    else
        qtlibs="Qt6Core Qt6Gui Qt6Widgets Qt6PrintSupport"
    fi
    for file in $qtlibs
    do
        cp -f $1/$3/bin/${file}${postfix}.dll $targetdir
    done
    if [ ! -d $targetdir/platforms ]; then
        mkdir $targetdir/platforms
    fi
    for file in qdirect2d qminimal qoffscreen qwindows
    do
        cp -f $1/$3/plugins/platforms/${file}${postfix}.dll $targetdir/platforms
    done
    if [ ! -d $targetdir/styles ]; then
        mkdir $targetdir/styles
    fi
    if [[ "$qtmaver" = "6" && "$qtmiver" -ge 7 ]]; then
        qtstyles="qmodernwindowsstyle"
    else
        qtstyles="qwindowsvistastyle"
    fi
    for file in $qtstyles
    do
        cp -f $1/$3/plugins/styles/${file}${postfix}.dll $targetdir/styles
    done
    if [ "$qtmaver" = "5" ]; then
        if [ ! -d $targetdir/printsupport ]; then
            mkdir $targetdir/printsupport
        fi
        for file in windowsprintersupport
        do
            cp -f $1/$3/plugins/printsupport/${file}${postfix}.dll $targetdir/printsupport
        done
    fi
    for file in flexemu.conf flexlabl.conf
    do
        cp -f src/$file $targetdir
    done
    if [ "x$5" != "x" ]; then
        # cmake only: copy built *.exe files.
        for file in flex2hex.exe hex2flex.exe dsktool.exe mdcrtool.exe flexemu.exe flexplorer.exe
        do
            cp -f $5/bin/$4/$file $targetdir
        done
    fi
    abspath=$( as_absolute_windows_path $targetdir )
    echo ==== to $abspath
}

is_cmake=false

while :
do
    case "$1" in
        -c) is_cmake=true;;
        --) shift; break;;
        -h) usage; exit 0;;
        *) break;;
    esac
    shift
done

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
check_value "$vsversion" "2019 2022"
if [ $? -ne 0 ]; then
    echo "**** File ${vsversion_file} has unsupported value for VS_TYPE."
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
qtbasedir=`sed -ne "s/ *<QTDIR>\(.*\)<\/QTDIR>/\1/p" $propsfile`
QTDIR=$( as_mingw_path "$qtbasedir")
qtversion=`sed -ne "s/ *<QTVERSION>\(.*\)<\/QTVERSION>/\1/p" $propsfile`
qtmaversion=`echo $qtversion | sed -e "s/\([56]\).*/\1/"`
qtmiversion=`echo $qtversion | sed -e "s/[0-9]\+.\([0-9]\+\).*/\1/"`

temp=$QTDIR/x64/bin
if [ ! -d $temp ]; then
    echo "**** Qt libraries build directory does not exist:"
    echo "**** $temp"
    echo "**** First call download_and_rebuild_libs.sh to create Qt libraries!"
    exit 1
fi

echo "Using Qt V${qtversion}"
if [ $is_cmake == true ]; then
    cmake_version=` cmake --version | sed -n "s/.*\([0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/p;q"`
    echo "Using cmake V${cmake_version}"
    echo
fi

cd ../..
vsmsbuilddir=$( as_mingw_path "$VSMSBUILDDIR")

# Prepare for cmake build.
if [ "$is_cmake" == true ]; then
    case "$vsversion" in
        2019) generator="Visual Studio 16 2019";;
        2022) generator="Visual Studio 17 2022";;
    esac
fi

# 1. When using cmake setup cmake to create a flexemu solution file (*.sln)
# 2. Rebuild flexemu solution for all platforms and configurations.
# 3. Check existence of result files and copy them to corresponding
#    flexemu build directory.
if [ ! -d bin ]; then mkdir bin; fi
if [ ! -d bin/Qt${qtversion} ]; then mkdir bin/Qt${qtversion}; fi
if [ "$qtmaversion" = "5" ]; then
    platforms="Win32 x64"
else
    platforms="x64"
fi

exit_code=0
flexemu_sln_file=flexemu.sln
builddir=""
for platform in $platforms
do
    if [ ! -d bin/$platform ]; then mkdir bin/$platform; fi
    if [ "$is_cmake" == true ]; then
        builddir=build${platform}
        if [ -d $builddir ]; then
            rm -rf $builddir
        fi
        qtsubdir=${qtbasedir}\\${platform}
        cmake.exe -S . -B $builddir -G "$generator" -DCMAKE_PREFIX_PATH="$qtsubdir" -DFLEXEMU_QT_MAJOR_VERSION=$qtmaversion -A $platform
        if [[ $? != 0 ]]; then
            echo "**** cmake reported an error. Aborted."
            exit 1
        fi
        flexemu_sln_file=${builddir}/Flexemu.sln
    fi
    for configuration in Debug Release
    do
        logfile=Build-flexemu-${platform}-${configuration}.log
        echo
        echo ==== Rebuild flexemu solution $platform $configuration ...
        echo For details see log-file "$logfile".
        "$vsmsbuilddir/MSBuild.exe" $flexemu_sln_file -t:Rebuild -m -v:quiet -clp:Summary -fl -flp:Verbosity=minimal\;LogFile=$logfile -property:Configuration=${configuration}\;Platform=${platform}
        if [[ $? != 0 ]]; then
            echo "**** There was a build error. Aborted."
            exit_code=1
        fi
        copy_files "$QTDIR" $qtversion $platform $configuration "$builddir"
    done
done
cd build/windows
exit $exit_code
