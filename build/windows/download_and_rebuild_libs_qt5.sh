#!/usr/bin/sh
#
# Download and rebuild Qt 5.x.y libraries needed to build flexemu
#
# syntax:
#    download_and_rebuild_libs_qt5.sh [-d][-V <vs_version][-T <vs_type>]
#                                     -v <qt_version>
# parameters:
#   -d:             Delete packages and directories before downloading
#                   and building them.
#   -V <vs_version> The Visual Studio version, 2017 or 2019.
#                   If not set the script looks for an installed VS version
#                   2019 or 2017, in this order.
#   -T <vs_type>    The Visual Studio type, Enterprise, Professional or
#                   Community. If not set the script looks for an installed VS
#                   type Enterprise, Professional or Community, in this order.
#   -v qt_version   Specify Qt version to be build.
#                   Syntax: <major>.<minor>.<patch>
#
# Prerequisites to build Qt on Windows see:
#    https://doc.qt.io/qt-5/windows-requirements.html
# Detailed instructions how to build Qt from Source see:
#    https://doc.qt.io/qt-5/windows-building.html
#
# Minumum requirements to sucessfully execute this script:
# - A recent ActivePerl installation from https://www.activestate.com/activeperl
# - Python 3.x from https://www.python.org/downloads/
# - CMake >= 3.15 from https://cmake.org/download/
# - jom.exe (nmake replacement) from https://wiki.qt.io/Jom

#Set all platforms to be build (Supported: Win32, x64)
platforms="Win32 x64"
# Appropriate Visual Studio versions and types:
vsversions="2019 2017"
vstypes="Enterprise Professional Community"

# DO NOT CHANGE ANYTHING BEYOND THIS LINE
#========================================

function usage() {
    echo "Download and rebuild Qt 5.x.y libraries needed to build flexemu"
    echo ""
    echo "Syntax:"
    echo "   download_and_rebuild_libs_qt5.sh [-d][-V <vs_version][-T <vs_type>]"
    echo "                                    -v <qt_version>"
    echo "Options:"
    echo "   -d:             Delete Qt downloads and build directories before downloading"
    echo "                   and building them"
    echo "   -V <vs_version> The Visual Studio version, 2017 or 2019."
    echo "                   If not set the script looks for an installed VS version"
    echo "                   2019 or 2017, in this order."
    echo "   -T <vs_type>    The Visual Studio type, Enterprise, Professional or"
    echo "                   Community. If not set the script looks for an installed VS"
    echo "                   type Enterprise, Professional or Community, in this order."
    echo "   -v <qt_version> Specify Qt version to be build. Syntax: 5.<minor>.<patch>"
}

qtversion=
delete=
vsversion=
vstype=
while :
do
    case "$1" in
        -d) delete=yes;;
        --) shift; break;;
        -h) usage; exit 0;;
        -v)
            if [ -n "$2" ]; then
                qtversion=$2
                shift 2
            else
                echo "Error: Argument for $1 is missing" >&2
                exit 1
            fi;;
        -V)
            if [ -n "$2" ]; then
                vsversion=$2
                shift
            else
                echo "Error: Argument for $1 is missing" >&2
                exit 1
            fi;;
        -T)
            if [ -n "$2" ]; then
                vstype=$2
                shift
            else
                echo "Error: Argument for $1 is missing" >&2
                exit 1
            fi;;
        *) break;;
    esac
    shift
done

if [ "x$qtversion" == "x" ]; then
    echo "Error: Qt version has to be specified with -v 5.<minor>.<patch>." >&2
    usage
    exit 1
fi
match=`echo $qtversion | sed -n "s/^\([5]\+\.[0-9]\+\.[0-9]\+\)$/\1/p"`
if [ "x$match" == "x" ]; then
    echo "Error: Qt version '$qtversion' has invalid syntax. Must be 5.<minor>.<patch>" >&2
    usage
    exit 1
fi
qtversion=$match
qtmamiversion=`echo $qtversion | sed -e "s/\([0-9]\+\.[0-9]\+\).*/\1/"`

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

# Look for an appropriate Visual Studio installation to set all needed
# variables:
check_value "$vsversion" "$vsversions"
if [ $? -ne 0 ]; then
    echo "*** Error: Visual Studio version \"$vsversion\" is not supported."
    usage
    exit 1
fi

check_value "$vstype" "$vstypes"
if [ $? -ne 0 ]; then
    echo "*** Error: Visual Studio type \"$vstype\" is not supported."
    usage
    exit 1
fi

if [ "x$vstype" == "x" ]; then
    if [ "x$vsversion" == "x" ]; then
        # No vstype or vsversion specified, look for it.
        for vsversion in $vsversions
        do
            for vstype in $vstypes
            do
                msvcscript="C:\Program Files (x86)\Microsoft Visual Studio\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
                if [ -f "$msvcscript" ]; then
                    break
                fi
            done
            if [ -f "$msvcscript" ]; then
                break
            fi
        done
    else
        # vsversion specified, look for vstype.
        for vstype in $vstypes
        do
            msvcscript="C:\Program Files (x86)\Microsoft Visual Studio\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
            if [ -f "$msvcscript" ]; then
                break
            fi
        done
    fi
else
    if [ "x$vsversion" == "x" ]; then
        # vstype specified, look for vsversion.
        for vsversion in $vsversions
        do
            msvcscript="C:\Program Files (x86)\Microsoft Visual Studio\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
            if [ -f "$msvcscript" ]; then
                break
            fi
        done
    else
        # both vstype and vsversion specified.
        msvcscript="C:\Program Files (x86)\Microsoft Visual Studio\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
    fi
fi

if [ -f "$msvcscript" ]; then
    echo "Found Visual Studio $vsversion $vstype"
else
    echo "*** Error: No appropriate Visual Studio Installation found."
    usage
    exit 1
fi

# Create the url from which to download a specific version (Supported: Qt5.minor.patch)
qturl=`echo "https://download.qt.io/archive/qt/${qtmamiversion}/${qtversion}/submodules/qtbase-everywhere-src-${qtversion}.zip"`

MSBUILDDISABLENODEREUSE=1
export MSBUILDDISABLENODEREUSE

builddir="build${qtversion}"
tgtdir="Qt${qtversion}"

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

# Convert a Windows path by doubling any backslash character.
as_doublebslash_windows_path() {
    path=`echo $1 | sed 's:\([\\]\):\1\1:g'`
    echo $path
}

# Create the Qt build batch script.
# $1: Batch script path setting all MSVC variables
# $2: Architecture (x86 or amd64)
# $3: Root path of the Qt source distribution (absolute Windows path)
# $4: Path Qt build directory (absolute Windows path)
# $5: Path of the generated batch script
create_qt_build_script() {
    echo CALL \"$1\" $2 >$5
    echo IF %ERRORLEVEL% neq 0 EXIT /b %ERRORLEVEL% >>$5
    echo SET _ROOT=$3 >>$5
    echo SET PATH=\%_ROOT\%\\bin\;\%PATH\% >>$5
    echo SET _ROOT= >>$5
    echo >>$5
    echo cd \"$4\" >>$5
    echo CALL \"$3\\configure.bat\" -redo >>$5
    echo IF %ERRORLEVEL% neq 0 EXIT /b %ERRORLEVEL% >>$5
    echo jom.exe >>$5
    echo IF %ERRORLEVEL% neq 0 EXIT /b %ERRORLEVEL% >>$5
    echo jom.exe install >>$5
    echo IF %ERRORLEVEL% neq 0 EXIT /b %ERRORLEVEL% >>$5
}

# Create the Qt build config file.
# $1: The target directory where the build artefacts are copied
# $2: Path to the config file
create_config_file() {
    echo "-platform" > $2
    echo "win32-msvc2019" >> $2
    echo "-prefix" >> $2
    echo "$1" >>$2
    echo "-debug-and-release" >>$2
    echo "-no-feature-network" >>$2
    echo "-no-feature-sql" >>$2
    echo "-no-feature-testlib" >>$2
    echo "-no-feature-concurrent" >>$2
    echo "-no-feature-dbus" >>$2
    echo "-no-feature-xml" >>$2
    echo "-nomake" >>$2
    echo "examples" >>$2
    echo "-qt-zlib" >>$2
    echo "-no-harfbuzz" >>$2
    echo "-no-opengl" >>$2
    echo "-c++std" >>$2
    echo "c++11" >>$2
    echo "-mp" >>$2
    echo "-confirm-license" >>$2
    echo "-opensource" >>$2
}

curlpath=`which curl 2>/dev/null`
if [ "x$curlpath" = "x" ]; then
    echo "*** Error: curl not found."
    echo "  curl can be downloaded from"
    echo "  https://curl.haxx.se/download.html"
    echo "  The executable has to be copied into the git installation"
    echo "  mingw32/bin or mingw64/bin."
fi

urls=`echo "$qturl"`
qtdir=Qt
if [ ! -d $qtdir ]; then
    mkdir $qtdir;
fi

# Option -d: delete previously downloaded packages and intermediate directories
if [ "$delete" = "yes" ]; then
    echo deleting all...
    for url in $urls
    do
        file=$(basename "$url")
        if [ -r $qtdir/$file ]; then
            echo deleting file $qtdir/$file...
            rm -f $qtdir/$file
        fi
        # evaluate directory name by cutting off the file extension (e.g. zip)
        filebase=`echo "$file" | sed -e "s/\(.\+\)\.\(zip\|tar.gz\)/\1/"`
        directory=$filebase
        if [ -d $qtdir/$directory ]; then
            echo deleting directory $qtdir/$directory...
            rm -rf $qtdir/$directory
        fi
    done
    if [ -d $qtdir/$builddir ]; then
        echo deleting directory $qtdir/$builddir...
        rm -rf $qtdir/$builddir
    fi
    if [ -d $qtdir/$tgtdir ]; then
        echo deleting directory $qtdir/$tgtdir...
        rm -rf $qtdir/$tgtdir
    fi
 fi

# Download files (Only if package not already downloaded or deleted before)
for url in $urls
do
    file=$(basename "$url")
    if [ ! -r $qtdir/$file ]; then
        echo downloading $file...
        curl -# -L $url > "$qtdir/$file"
    fi
done
if [ ! "$?" == "0" ]; then
    echo "Error: Download failed. Aborted." >&2
    rm -f $qtdir/$file
    exit 1
fi

# Unpacking files
# Supported extensions: tar.gz or zip
qtsrcdir=
for url in $urls
do
    file=$(basename "$url")
    if [ -r $qtdir/$file ]; then
        extension=`echo "$file" | sed -e "s/.\+\(zip\|tar.gz\)/\1/"`
        filebase=`echo "$file" | sed -e "s/\(.\+\)\.\(zip\|tar.gz\)/\1/"`
        directory=$filebase
        if [ "$url" == "$qturl" ]; then
            qtsrcdir=$qtdir/$directory
        fi
        if [ ! -d $qtdir/$directory ]; then
            echo unpacking $file into $qtdir...
            case "$extension" in
                tar.gz) tar -C $qtdir xfz $qtdir/$file ;;
                zip) unzip -q $qtdir/$file -d $qtdir ;;
            esac
            if [ ! "$?" == "0" ]; then
                echo "Error: Unpacking file failed. Aborted." >&2
                exit 1
            fi
            # Wait for 2 seconds otherwise the mv could fail
            sleep 2
        fi
    fi
done

if [ ! -d $qtdir/$builddir ]; then
    mkdir $qtdir/$builddir;
fi

if [ ! -d $qtdir/$tgtdir ]; then
    mkdir $qtdir/$tgtdir;
fi

absqttgtdir=$( as_absolute_windows_path $qtdir/$tgtdir )

# Create batch script to build Qt libraries for
# all requested platforms and execute it.
for platform in $platforms
do
    directory="${builddir}/${platform}"
    if [ ! -d $qtdir/$directory ]; then
        mkdir $qtdir/$directory;
    fi
    arch=x86
    if [ "x$platform" = "xx64" ]; then
        arch=amd64;
    fi

    configoptpath=$qtdir/$directory/config.opt
    create_config_file $absqttgtdir\\$platform $configoptpath
    absqtsrcdir=$( as_absolute_windows_path $qtsrcdir )
    absqtbuilddir=$( as_absolute_windows_path $qtdir/$directory )
    batchscript=$qtdir/$directory/build_qt.bat
    create_qt_build_script "$msvcscript" $arch "$absqtsrcdir" "$absqtbuilddir" $batchscript

    echo building $platform Qt $qtversion libraries...
    cmd < $batchscript
    if [ ! "$?" == "0" ]; then
        echo "Error: Building Qt libraries. Aborted." >&2
        exit 1
    fi
done

file=../../src/msvcQtPath.props
cp -f ${file}.in $file
path=$( as_doublebslash_windows_path $absqttgtdir )
expr="s/QTDIRPLACEHOLDER/$path/"
sed -i -e $expr $file

echo ""
echo "Finished successfully. Next step is to call"
echo rebuild_flexemu_all_and_copy_libs.sh
