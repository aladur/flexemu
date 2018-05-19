#!/usr/bin/sh
#
# Create a windows installer executable.
# If needed the VC redistributable packages are downloaded.
#
# syntax:
#    create_installer.sh [-d]
# parameters:
#   -d:    delete packages before downloading them
#

delete=
modified=false

while :
do
    case "$1" in
    	-d) delete=yes;;
    	--) shift; break;;
    	*) break;;
    esac
    shift
done

# curl is needed to execute this script.
curlpath=`which curl 2>/dev/null`
if [ "x$curlpath" = "x" ]; then
    echo "*** Error: curl not found."
    echo "  curl can be downloaded from:"
    echo "     https://curl.haxx.se/download.html"
    echo "  The executable has to be copied into the git installation"
    echo "  mingw32/bin or mingw64/bin."
	exit
fi

# nsis installer is needed to execute this script.
nsispath=`which makensis 2>/dev/null`
if [ "x$nsispath" = "x" ]; then
    echo "*** Error: NSIS is not installed."
    echo "  NSIS windows installer can be downloaded from:"
    echo "     http://nsis.sourceforge.net/Download"
	exit
fi

# URL from where to get the download location of vc_redist packages for VS2015/VS2017:
# https://github.com/WPN-XM/vcredist
urls="https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x86.exe \
      https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x64.exe"

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
fi

# Download files (Only if package not already downloaded or deleted before)
for url in $urls
do
    file=$(basename "$url")
    if [ ! -r $file ]; then
        curl -# -L $url > "${file}"
    fi
done

# Calling NSIS to create the windows installer executable.
# makensis enters the installer directory so installer is
# the current working directory when executing Flexemu.nsi.
makensis installer\\Flexemu.nsi