#!/usr/bin/sh
#
# syntax:
#    rebuild_all_and_copy_libs.sh [-d]
# parameters:
#    none
#

create_rebuild_script() {
    echo \@echo off >$2
    echo set DevEnvDir=$VS140COMNTOOLS\..\\IDE >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Release\|x64\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|Win32\" >>$2
    echo \"\%DevEnvDir%\\devenv.exe\" $1 /Rebuild \"Debug\|x64\" >>$2
}

if [ ! -d lib ]; then
    echo "**** First call download_and_rebuild_libs.sh to create libraries!"
    exit 1
fi

cd ../..
# Create and execute a batch file to build all four configurations
create_rebuild_script flexemu.sln rebuild.bat
echo build flexemu...
#$COMSPEC //C rebuild.bat

# check existence of result files and copy them to corresponding
# flexemu build directory

echo copy libraries...
for platform in Win32 x64
do
    if [ ! -d bin/$platform ]; then mkdir bin/$platform; fi
    for configuration in Debug Release
    do
        if [ ! -d bin/$platform/$configuration ]; then
            mkdir bin/$platform/$configuration
        fi
        cp -f build/windows/lib/${platform}/${configuration}/*.dll bin/$platform/$configuration
    done
done
cd build/windows
