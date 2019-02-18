#!/usr/bin/sh
#
# Create a windows installer executable for all
# FLEX user group DSK files.
#
# syntax:
#    create_fug_dsk_files_installer.sh [-d]
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

# nsis installer is needed to execute this script.
nsispath=`which makensis 2>/dev/null`
if [ "x$nsispath" = "x" ]; then
    echo "*** Error: NSIS is not installed."
    echo "  NSIS windows installer can be downloaded from:"
    echo "     http://nsis.sourceforge.net/Download"
	exit
fi

directory=fug_dsk
# Option -d: delete previously downloaded DSK files
if [ "$delete" = "yes" ]; then
    echo deleting all...
    if [ -d $directory ]; then
        echo deleting $directory...
        rm -rf $directory
    fi
fi

../download_fug_dsk_files.sh -D ../fug_dsk_files

# Calling NSIS to create the windows installer executable.
# makensis enters the installer directory so installer is
# the current working directory when executing HistoricFLEXFiles.nsi.
makensis installer\\HistoricFLEXFiles.nsi
