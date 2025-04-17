#!/usr/bin/sh
#
# Download and install NSIS EnVar plugin.
#
# syntax:
#    download_and_install_envar.sh [-d <target_directory>][-s]
#    download_and_install_envar.sh -h
#
# parameters:
#   -d <target_dir> Defines the directory where to install the plugin.
#   -s              Suppress progress bar when downloading files.
#   -h              Print this help and exit.
#
function usage()
{
    echo "Download and install NSIS EnVar plugin"
    echo ""
    echo "Syntax:"
    echo "   download_and_install_envar.sh -d <target_directory>[-s]"
    echo "   download_and_install_envar.sh -h"
    echo ""
    echo "Options:"
    echo "   -d <target_dir>:  Defines the directory where to install the plugin."
    echo "   -s                Suppress progress bar when downloading files."
    echo "   -h                Print this help and exit."
}

target_dir=""
curl_progress=""

while :
do
    case "$1" in
        --) shift; break;;
        -h) usage; exit 0;;
        -d)
            if [ -n "$2" ]; then
                target_dir=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        -s)
            curl_progress="-sS";;
        *) break;;
    esac
    shift
done

if [ "x$target_dir" = "x" ]; then
    echo "*** Error: target directory has to be specified (parameter -d)."
    usage
    exit 1
fi
if [ ! -d $$target_dir ]; then
    mkdir -p $target_dir;
fi

curlpath=`which curl 2>/dev/null`
if [ "x$curlpath" = "x" ]; then
    echo "*** Error: curl not found."
    echo "  curl can be downloaded from"
    echo "  https://curl.haxx.se/download.html"
    echo "  The executable has to be copied into the git installation"
    echo "  mingw32/bin or mingw64/bin."
    exit 1
fi

urls="https://github.com/GsNSIS/EnVar/releases/download/v0.3.1/EnVar-Plugin.zip"
for url in $urls
do
    file=$(basename "$url")
    if [ ! -r $file ]; then
        echo downloading $file...
        curl $curl_progress -# -L $url --remove-on-error \
            --max-time 10 --retry 5 --retry-delay 0 \
            --retry-max-time 40 > "$file"
    fi
    if [ ! -r $file ]; then
        echo "*** Error: Downloading $file failed, aborted."
        exit 1
    fi
    echo extracting $file...
    unzip -q $file -d $target_dir
done
