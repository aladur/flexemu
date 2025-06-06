#!/usr/bin/sh
#
# Download and rebuild Qt x.y.z libraries needed to build flexemu
#
# syntax:
#    download_and_rebuild_libs.sh [-d][-V <vs_version][-T <vs_type>]
#                                 [-s][-m <mirror>]-v <qt_version>
# parameters:
#   -d:             Delete packages and directories before downloading
#                   and building them.
#   -V <vs_version> The Visual Studio version, 2019 or 2022.
#                   If not set the script looks for an installed VS version
#                   2019 or 2022, in this order. Visual Studio has to be 
#                   installed in the default path.
#   -T <vs_type>    The Visual Studio type, Enterprise, Professional or
#                   Community. If not set the script looks for an installed VS
#                   type Enterprise, Professional or Community, in this order.
#   -v qt_version   Specify Qt version to be build.
#                   Syntax: <major>.<minor>.<patch>
#                   Supported major version is 5 or 6.
#                   See https://download.qt.io/archive/qt/ for available versions.
#   -s              Suppress progress bar when downloading files.
#   -m <mirror>     Use mirror site base url for download. It should contain
#                   an 'archive' folder. Default is the Qt download site:
#                   https://download.qt.io.
#
# Qt5
#=====
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
#
# Qt6
#=====
# Prerequisites to build Qt on Windows see:
#    https://doc.qt.io/qt-6/windows.html
# Detailed instructions how to build Qt from Source see:
#    https://doc.qt.io/qt-6/windows-building.html
#
# Minumum requirements to sucessfully execute this script:
# - A recent ActivePerl installation from https://www.activestate.com/activeperl
# - Python 3.x from https://www.python.org/downloads/
# - CMake >= 3.17 from https://cmake.org/download/
# - ninja.exe (nmake replacement) from https://ninja-build.org/

# Appropriate Visual Studio versions and types:
vsversions="2019 2022"
vstypes="Enterprise Professional Community"

# DO NOT CHANGE ANYTHING BEYOND THIS LINE
#========================================

function usage() {
    echo "Download and rebuild Qt x.y.z libraries needed to build flexemu"
    echo ""
    echo "Syntax:"
    echo "   download_and_rebuild_libs.sh [-d][-V <vs_version][-T <vs_type>]"
    echo "                                [-s][-m <mirror>]-v <qt_version>"
    echo "Options:"
    echo "   -d:             Delete Qt downloads and build directories before downloading"
    echo "                   and building them"
    echo "   -V <vs_version> The Visual Studio version, 2019 or 2022."
    echo "                   If not set the script looks for an installed VS version"
    echo "                   2019 or 2022, in this order. Visual Studio has to be"
    echo "                   installed in the default path."
    echo "   -T <vs_type>    The Visual Studio type, Enterprise, Professional or"
    echo "                   Community. If not set the script looks for an installed VS"
    echo "                   type Enterprise, Professional or Community, in this order."
    echo "   -v <qt_version> Specify Qt version to be build."
    echo "                   Syntax: <major>.<minor>.<patch>"
    echo "                   Supported major version is 5 or 6. See"
    echo "                   https://download.qt.io/archive/qt/ for available versions."
    echo "   -s              Suppress progress bar when downloading files."
    echo "   -m <mirror>     Use mirror site base url for download. It should contain"
    echo "                   an 'archive' folder. Default is the Qt download site:"
    echo "                   https://download.qt.io"
}

qtversion=
delete=
vsversion=
vstype=
curl_progress=""
baseurl="https://download.qt.io"

while :
do
    case "$1" in
        -d) delete=yes;;
        --) shift; break;;
        -h) usage; exit 0;;
        -v)
            if [ -n "$2" ]; then
                qtversion=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        -V)
            if [ -n "$2" ]; then
                vsversion=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        -T)
            if [ -n "$2" ]; then
                vstype=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        -s)
            curl_progress="-sS";;
        -m)
            if [ -n "$2" ]; then
                baseurl=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        *) break;;
    esac
    shift
done

if [ "x$qtversion" == "x" ]; then
    echo "**** Error: Qt version has to be specified with"
    echo "**** -v <major>.<minor>.<patch>." >&2
    usage
    exit 1
fi
match=`echo $qtversion | sed -n "s/^\([56]\.[0-9]\+\.[0-9]\+\)$/\1/p"`
if [ "x$match" == "x" ]; then
    echo "**** Error: Qt version '$qtversion' has invalid syntax."
    echo "**** Must be <5|6>.<minor>.<patch>" >&2
    usage
    exit 1
fi
qtversion=$match
qtmamiversion=`echo $qtversion | sed -e "s/\([56]\.[0-9]\+\).*/\1/"`
qtmaversion=`echo $qtversion | sed -e "s/\([56]\).*/\1/"`
qtmiversion=`echo $qtversion | sed -e "s/^[56]\.\([0-9]\+\)\.[0-9]\+$/\1/"`
qtpatch=`echo $qtversion | sed -e "s/^[56]\.[0-9]\+\.\([0-9]\+\)$/\1/"`

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

# Get the base path of the Microsoft Visual Studio installation.
# $1: Microsoft Visual Studio version ($vsversion)
get_vsbasedir() {
    if [ "$1" == "2019" ]; then
        echo "C:\Program Files (x86)\Microsoft Visual Studio"
    else
        echo "C:\Program Files\Microsoft Visual Studio"
    fi
}

# Look for an appropriate Visual Studio installation to set all needed
# variables:
check_value "$vsversion" "$vsversions"
if [ $? -ne 0 ]; then
    echo "**** Error: Visual Studio version \"$vsversion\" is not supported."
    usage
    exit 1
fi

check_value "$vstype" "$vstypes"
if [ $? -ne 0 ]; then
    echo "**** Error: Visual Studio type \"$vstype\" is not supported."
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
                vsbasedir=$( get_vsbasedir $vsversion )
                msvcscript="$vsbasedir\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
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
            vsbasedir=$( get_vsbasedir $vsversion )
            msvcscript="$vsbasedir\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
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
            vsbasedir=$( get_vsbasedir $vsversion )
            msvcscript="$vsbasedir\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
            if [ -f "$msvcscript" ]; then
                break
            fi
        done
    else
        # both vstype and vsversion specified.
        vsbasedir=$( get_vsbasedir $vsversion )
        msvcscript="$vsbasedir\\$vsversion\\$vstype\VC\Auxiliary\Build\vcvarsall.bat"
    fi
fi

baseurl=`echo $baseurl | sed "s|/$||"`
vsversion_file="vsversion.ini"
echo "VS_VERSION=$vsversion" > $vsversion_file
echo "VS_TYPE=$vstype" >> $vsversion_file

if [ -f "$msvcscript" ]; then
    echo "Found Visual Studio $vsversion $vstype"
else
    echo "**** Error: No appropriate Visual Studio Installation found."
    echo "**** Microsoft Visual Studio has to be installed in the default path."
    usage
    exit 1
fi

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
# $5: Qt major version
# $6: Path of the generated batch script
create_qt_build_script() {
    echo CALL \"$1\" $2 >$6
    echo IF %ERRORLEVEL% neq 0 GOTO :end >>$6
    echo SET _ROOT=$3 >>$6
    echo SET PATH=\%_ROOT\%\\bin\;\%PATH\% >>$6
    echo SET _ROOT= >>$6
    echo cd \"$4\" >>$6
    echo CALL \"$3\\configure.bat\" -redo >>$6
    echo IF %ERRORLEVEL% neq 0 GOTO :end >>$6
if [ "$5" = "5" ]; then
    echo jom.exe >>$6
    echo IF %ERRORLEVEL% neq 0 GOTO :end >>$6
    echo jom.exe install >>$6
else
    echo cmake --build . --parallel >>$6
    echo IF %ERRORLEVEL% neq 0 GOTO :end >>$6
    echo ninja.exe install >>$6
fi
    echo :end >>$6
    echo echo ERRORLEVEL=\%ERRORLEVEL\% >>$6
    echo EXIT /b %ERRORLEVEL% >>$6
    chmod +x $6
}

# Create the Qt build config file.
# $1: The target directory where the build artefacts are copied
# $2: Qt major version
# $3: Path to the config file
# Remark: Some more features have been activated to reduce the
# chance of an internal compiler error.
create_config_file() {
    echo "-platform" > $3
    echo "win32-msvc" >> $3
    echo "-prefix" >> $3
    echo "$1" >>$3
    echo "-debug-and-release" >>$3
    echo "-feature-network" >>$3
    echo "-feature-sql" >>$3
    echo "-no-feature-testlib" >>$3
    echo "-feature-concurrent" >>$3
    echo "-feature-dbus" >>$3
    echo "-feature-xml" >>$3
    echo "-nomake" >>$3
    echo "examples" >>$3
    echo "-qt-zlib" >>$3
    echo "-qt-harfbuzz" >>$3
    echo "-opengl" >>$3
    echo "desktop" >>$3
    # Avoid internal compiler error on Qt5, Qt6
    echo "-no-pch" >>$3
    echo "-c++std" >>$3
if [ "$2" = "5" ]; then
    echo "c++11" >>$3
else
    echo "c++17" >>$3
fi
if [ "$2" = "5" ]; then
    # Option to use multiple cores is only supported on Qt5
    echo "-mp" >>$3
fi
    echo "-confirm-license" >>$3
    echo "-opensource" >>$3
}

curlpath=`which curl 2>/dev/null`
if [ "x$curlpath" = "x" ]; then
    echo "*** Error: curl not found."
    echo "  curl can be downloaded from"
    echo "  https://curl.haxx.se/download.html"
    echo "  The executable has to be copied into the git installation"
    echo "  mingw32/bin or mingw64/bin."
fi

if [ "$qtmaversion" = "5" ]; then
    jompath=`which jom 2>/dev/null`
    if [ "x$jompath" = "x" ]; then
        echo "**** Error: jom not found."
        echo "  Jom can be downloaded from"
        echo "  https://wiki.qt.io/Jom"
        echo "  The executable has to be copied into PATH."
        exit 1
    fi
else
    ninjapath=`which ninja 2>/dev/null`
    if [ "x$ninjapath" = "x" ]; then
        echo "**** Error: ninja not found."
        echo "  ninja can be downloaded from"
        echo "  https://ninja-build.org/"
        echo "  The executable has to be copied into PATH."
        exit 1
    fi
fi

# Create the url from which to download a specific version
# (Supported: Qt(5|6).minor.patch)

qtos=""
# In qtosversions define an array with major.minor versions.
# In qtospatchver define an array with the minimum patch version,
# for which to use the "-opensource" substring.
qtosversions=("5.15" "6.2" "6.5")
qtospatchver=( 3      5     4   )
for (( i=0; i<${#qtosversions[@]}; i++ ))
do
    if [ "${qtosversions[$i]}" == "$qtmamiversion" ] && [ $qtpatch -ge ${qtospatchver[$i]} ]; then
        qtos="-opensource"
        break
    fi
done

# There is a default name containing all md5sums of a directory.
# For some versions it has a different name.
md5sumfile='md5sums.txt'
check_value "$qtversion" "6.2.5 5.15.3"
if [ $? -eq 0 ]; then
    md5sumfile='md5sum.txt'
fi

# Since January 2025 the files are located in a "src" subdirectory.
# In qtsubdirversions define an array with major.minor versions.
# In qtsubdirpatchver define an array with the minimum patch version,
# for which to use the subdirectory "src".
subdir=""
qtsubdirversions=("6.2" "6.5")
qtsubdirpatchver=( 11    4   )
for (( i=0; i<${#qtsubdirversions[@]}; i++ ))
do
    if [ "${qtsubdirversions[$i]}" == "$qtmamiversion" ] && [ $qtpatch -ge ${qtsubdirpatchver[$i]} ]; then
        subdir="/src"
        break
    fi
done

qtfile=`echo "qtbase-everywhere${qtos}-src-${qtversion}.zip"`
qturl=`echo "https://download.qt.io/archive/qt/${qtmamiversion}/${qtversion}${subdir}/submodules/${qtfile}"`
md5sums=`echo "${baseurl}/archive/qt/${qtmamiversion}/${qtversion}${subdir}/submodules/${md5sumfile}"`

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
        file=`echo "$file" | sed -e "s/\(.\+\)-opensource\(.\+\)/\1\2/"`
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
# Execute a checksum validation.
for url in $urls
do
    file=$(basename "$url")
    if [ ! -r $qtdir/$file ]; then
        echo download site: $baseurl
        echo downloading $file...
        curl $curl_progress -# -f -L $url > "$qtdir/$file"
        if [ ! "$?" == "0" ]; then
            echo "**** Error: Download of $file failed. Aborted." >&2
            echo "**** URL: $url" >&2
            rm -f $qtdir/$file
            exit 1
        fi
        echo downloading $md5sumfile...
        curl $curl_progress -# -f -L -O --remove-on-error $md5sums
        if [ ! "$?" == "0" ]; then
            echo "**** Error: Download of $md5sumfile failed. Aborted." >&2
            echo "**** URL: $md5sums" >&2
            rm -f $qtdir/$file
            exit 1
        fi
        md5sum=`cat $md5sumfile | sed -n "s/\([0-9a-z]\+\) \+${qtfile}$/\1/p"`
        expected=`md5sum ${qtdir}/${qtfile} | sed "s/ .*//"`
        if [ "$md5sum" != "$expected" ]; then
            echo "Checksum error:"
            echo "  md5sum=  $md5sum"
            echo "  expected=$expected"
            exit 1
        else
            echo "Checksum verification passed."
        fi
        rm $md5sumfile
    fi
done

# Unpacking files
# Supported extensions: tar.gz or zip
qtsrcdir=
for url in $urls
do
    orgfile=$(basename "$url")
    file=`echo "$orgfile" | sed -e "s/\(.\+\)-opensource\(.\+\)/\1\2/"`
    if [ -r $qtdir/$orgfile ]; then
        extension=`echo "$file" | sed -e "s/.\+\(zip\|tar.gz\)/\1/"`
        filebase=`echo "$file" | sed -e "s/\(.\+\)\.\(zip\|tar.gz\)/\1/"`
        directory=$filebase
        if [ "$url" == "$qturl" ]; then
            qtsrcdir=$qtdir/$directory
        fi
        if [ ! -d $qtdir/$directory ]; then
            echo unpacking $orgfile into $qtdir...
            case "$extension" in
                tar.gz) tar -C $qtdir xfz $qtdir/$orgfile ;;
                zip) unzip -q $qtdir/$orgfile -d $qtdir ;;
            esac
            if [ ! "$?" == "0" ]; then
                echo "**** Error: Unpacking file failed. Aborted." >&2
                exit 1
            fi
            # Wait for 2 seconds otherwise the mv could fail
            sleep 2
        fi
    fi
done

# Since Qt 6.9.0 building Qt on VS2019 aborts with an error.
# See also:
# https://doc.qt.io/qt-6.9/windows.html
if [ "$qtmaversion" == "6" ] && [ ${qtmiversion} -ge 9 ] && [ "$vsversion" == "2019" ]; then
    echo "**** Error: Qt >= 6.9.0 can not be build with Visual Studio 2019." >&2
    exit 2
fi

# Since Qt 6.8.0 only MSVC 2022 is supported.
# A cmake flag -DQT_NO_MSVC_MIN_VERSION_CHECK:BOOL=ON
# can be set to avoid the compiler version check to still
# build with MSVC 2019. So these are inofficial versions.
# See also:
# https://doc.qt.io/qt-6.8/windows.html
# https://doc.qt.io/qt-6.7/windows.html
# https://www.qt.io/blog/moving-to-msvc-2022-in-qt-68
# Enable cmake flag by patching a cmake file.
if [ "$qtmaversion" == "6" ] && [ ${qtmiversion} -ge 8 ] && [ "$vsversion" == "2019" ]; then
    filetopatch=${qtsrcdir}/cmake/QtProcessConfigureArgs.cmake
    count=`sed -n "/push.*QT_NO_MSVC_MIN_VERSION_CHECK/p" $filetopatch | wc -l`
    if [ $count -eq 0 ]; then
        sed -i '/DQT_INTERNAL_CALLED_FROM_CONFIGURE:BOOL=TRUE/a push("-DQT_NO_MSVC_MIN_VERSION_CHECK=ON")' $filetopatch
    fi
    echo "==== REMARK: Building Qt ${qtversion} with Visual Studio ${vsversion} is an inofficial version."
fi

if [ ! -d $qtdir/$builddir ]; then
    mkdir $qtdir/$builddir;
fi

if [ ! -d $qtdir/$tgtdir ]; then
    mkdir $qtdir/$tgtdir;
fi

absqttgtdir=$( as_absolute_windows_path $qtdir/$tgtdir )

# Get all supported platforms to be build (Supported: Win32, x64).
# They depend on the Qt major version
if [ "$qtmaversion" == "5" ]; then
    # Supported on Qt5: Win32, x64
    platforms="Win32 x64"
else
    # Supported on Qt6: x64
    platforms="x64"
fi

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
    create_config_file $absqttgtdir\\$platform $qtmaversion $configoptpath
    absqtsrcdir=$( as_absolute_windows_path $qtsrcdir )
    absqtbuilddir=$( as_absolute_windows_path $qtdir/$directory )
    batchscript=$qtdir/$directory/build_qt.cmd
    create_qt_build_script "$msvcscript" $arch "$absqtsrcdir" "$absqtbuilddir" $qtmaversion $batchscript
    echo ========== START Contents of build batch file ==========
    cat $batchscript
    echo ========== END Contents of build batch file ==========

    echo building $platform Qt $qtversion libraries...
    ./$batchscript
    if [ ! "$?" == "0" ]; then
        echo "**** Error: Building Qt libraries. Aborted." >&2
        exit 1
    fi
done

file=../../src/msvcQtPath.props
cp -f ${file}.in $file
path=$( as_doublebslash_windows_path $absqttgtdir )
expr="s/QTDIRPLACEHOLDER/$path/"
sed -i -e $expr $file
expr="s/QTVERSIONPLACEHOLDER/$qtversion/"
sed -i -e $expr $file
expr="s/QTMAJORPLACEHOLDER/$qtmaversion/"
sed -i -e $expr $file

echo ""
echo "Finished successfully. Next step is to call"
echo rebuild_flexemu_all_and_copy_libs.sh
