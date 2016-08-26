#!/usr/bin/sh
#
# syntax:
#    download_and_rebuild_libs.sh [-d]
# parameters:
#   -d:    delete packages before downloading them
#

delete=
modified=false
MSBUILDDISABLENODEREUSE=1
export MSBUILDDISABLENODEREUSE

create_rebuild_dll_script() {
    echo \@echo off >$2
    echo set DevEnvDir=$VS140COMNTOOLS\..\\IDE >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"DLL Release\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"DLL Release\|x64\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"DLL Debug\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"DLL Debug\|x64\" >>$2
}

while :
do
    case "$1" in
    	-d) delete=yes;;
    	--) shift; break;;
    	*) break;;
    esac
    shift
done

curlpath=`which curl 2>/dev/null`
if [ "x$curlpath" = "x" ]; then
    echo "*** Error: curl not found."
    echo "  curl can be downloaded from"
    echo "  https://curl.haxx.se/download.html"
    echo "  The executable has to be copied into the git installation"
    echo "  mingw32/bin or mingw64/bin."
fi

urls="https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.0/wxWidgets-3.1.0.zip"


# Option -d: delete previously downloaded packages and intermediate directories
if [ "$delete" = "yes" ]; then
    echo deleting all...
    for url in $urls
    do
        file=$(basename "$url")
        if [ -r $file ]; then
            echo deleting $file...
            rm -f $file
        fi
        directory=`echo "$file" | sed -e "s/\([A-Za-z0-9_]\+\)-.*/\1/"`
        if [ -d $directory ]; then
            echo deleting $directory...
            rm -rf $directory
        fi
    done
    rm -rf lib
    rm -rf libiconv
fi

# Download files (Only if package not already downloaded or deleted before)
for url in $urls
do
    file=$(basename "$url")
    if [ ! -r $file ]; then
        curl -# -L $url > "${file}"
    fi
done

# Unpacking files
# Supported extensions: tar.gz or zip
for url in $urls
do
    file=$(basename "$url")
    if [ -r $file ]; then
       extension=`echo "$file" | sed -e "s/.\+\(zip\|tar.gz\)/\1/"`
       filebase=`echo "$file" | sed -e "s/\(.\+\)\.\(zip\|tar.gz\)/\1/"`
       directory=`echo "$file" | sed -e "s/\([A-Za-z0-9_]\+\)-.*/\1/"`
	   
       if [ ! -d $directory ]; then
           echo unpacking $file...
           case "$extension" in
               tar.gz) tar xfz $file ;;
               zip) unzip $file -d $directory ;;
           esac
           # Wait for 2 seconds otherwise the mv could fail
           sleep 2
       fi
    fi
done

# wxWidgets modify project and compile with VS2015:
if [ -d wxWidgets ]; then
    cd wxWidgets/build/msw
    if [ ! -r vc_mswudll ]; then
        modified=true
        # Create and execute a batch file to build all four configurations
        create_rebuild_dll_script wx_vc14.sln rebuild.bat
        echo build wxWidgets...
        $COMSPEC //C rebuild.bat
    fi
    cd ../../..
fi

if [ "$modified" = true ]; then
    if [ -d lib ]; then rm -rf lib; fi
fi

if [ ! -d lib ]; then
    mkdir lib
    # check existence of result files and copy them to lib directory
    for platform in Win32 x64
    do
        if [ ! -d lib/$platform ]; then mkdir lib/$platform; fi
        for configuration in Debug Release
        do
            mkdir lib/$platform/$configuration
            echo config=$platform\|$configuration
			x64=""
			d=""
			if [ $configuration = Debug ]; then
				d="d"
			fi
			if [ $platform = x64 ]; then
				x64="_x64"
			fi
			srcdir="wxWidgets/lib/vc${x64}_dll"
			echo srcdir= $srcdir
			for file in wxbase31u${d}_vc${x64}_custom.dll wxmsw31u${d}_core_vc${x64}_custom.dll \
				wxbase31u${d}.lib wxmsw31u${d}_core.lib
			do
				if [ ! -f $srcdir/$file ]; then
					echo "   *** Missing file: $srcdir/$file"
				else
					echo "   copying $file..."
					cp -f    $srcdir/$file lib/${platform}/${configuration}/
				fi
			done
        done
    done
fi
