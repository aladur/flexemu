#!/usr/bin/bash
#
# Change the flexemu project version
#
# syntax:
#    change_version.sh -o <old_version> -v <new_version>
# parameters:
#   -o <old_version> Set the old version (which is to be overwritten)
#   -v <new_version> Set the new version (which is to be set)
# parameters:
#   <old_version>    <major>.<minor>.<patch>
#   <new_version>    <major>.<minor>.<patch>
#
# DO NOT CHANGE ANYTHING BEYOND THIS LINE
#========================================

function usage() {
    echo "Change the flexemu project version"
    echo ""
    echo "Syntax:"
    echo "   change_version.sh -o <old_version> -v <new_version>"
    echo "Options:"
    echo "   -o <old_version> Set the old version (which is to be overwritten)"
    echo "   -v <new_version> Set the new version (which is to be set)"
    echo "Parameters:"
    echo "   <old_version>    <major>.<minor>.<patch>"
    echo "   <new_version>    <major>.<minor>.<patch>"
}

while :
do
    case "$1" in
        --) shift; break;;
        -h) usage; exit 0;;
        -o)
            if [ -n "$2" ]; then
                old_version=$2
                shift
            else
                echo "**** Error: Argument for $1 is missing" >&2
                usage
                exit 1
            fi;;
        -v)
            if [ -n "$2" ]; then
                new_version=$2
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

if [ "x$old_version" == "x" ]; then
    echo "**** Error: Old version has to be specified with"
    echo "**** -o <major>.<minor>" >&2
    usage
    exit 1
fi

if [ "x$new_version" == "x" ]; then
    echo "**** Error: New version has to be specified with"
    echo "**** -v <major>.<minor>.<patch>" >&2
    usage
    exit 1
fi

old_major=`echo $old_version | sed -e "s/^\([0-9]\).*/\1/"`
old_minor=`echo $old_version | sed -ne "s/^[0-9]\.\([0-9]\+\).*/\1/p"`
old_patch=`echo $old_version | sed -e "s/^[0-9]\.[0-9]\+\.\([0-9]\+\).*/\1/"`
new_major=`echo $new_version | sed -e "s/^\([0-9]\).*/\1/"`
new_minor=`echo $new_version | sed -e "s/^[0-9]\.\([0-9]\+\).*/\1/"`
new_patch=`echo $new_version | sed -e "s/^[0-9]\.[0-9]\+\.\([0-9]\+\).*/\1/"`

# Create dot separated version number
old_version=${old_major}\\.${old_minor}\\.${old_patch}
new_version=${new_major}.${new_minor}.${new_patch}

# Create comma separated version number (for Windows rc-files)
old_cversion=${old_major},${old_minor},${old_patch},0
new_cversion=${new_major},${new_minor},${new_patch},0

echo changing version from ${old_major}.${old_minor}.${old_patch} to ${new_version}

sed -i "s/$old_version/$new_version/" installer/Flexemu.nsi
sed -i "s/$old_version/$new_version/" ../../src/confignt.h
sed -i "s/${old_version}\s*$/$new_version/" ../../CMakeLists.txt
sed -i "s/$old_cversion/$new_cversion/" ../../src/flexdisk.rc
sed -i "s/$old_version/$new_version/" ../../src/flexdisk.rc
sed -i "s/$old_cversion/$new_cversion/" ../../src/flexemu.rc
sed -i "s/$old_version/$new_version/" ../../src/flexemu.rc
sed -i "s/$old_cversion/$new_cversion/" ../../src/dsktool.rc
sed -i "s/$old_version/$new_version/" ../../src/dsktool.rc
sed -i "s/$old_cversion/$new_cversion/" ../../src/mdcrtool.rc
sed -i "s/$old_version/$new_version/" ../../src/mdcrtool.rc
sed -i "s/$old_cversion/$new_cversion/" ../../src/flex2hex.rc
sed -i "s/$old_version/$new_version/" ../../src/flex2hex.rc
sed -i "s/$old_cversion/$new_cversion/" ../../src/hex2flex.rc
sed -i "s/$old_version/$new_version/" ../../src/hex2flex.rc

# Reset pre-release to "-alpha"
sed -i "s/\(VERSION_PRERELEASE\)\s\+\".*\"/\1 \"-alpha\"/" ../../CMakeLists.txt
sed -i "s/\(VERSION_PRERELEASE\)\s\+\".*\"/\1 \"-alpha\"/" ../../src/confignt.h
