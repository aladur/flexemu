#!/usr/bin/env sh
#
# FLEX User Group - Historic Disk Collection downloader.
#
# It executes the following tasks:
# - Downloads all DSK files from FUG homepage.
# - Downloads the index HTML files from FUG homepage.
# - Removes DSK files which are not downloadable.
# - Fixes lower/upper case filename mismatches.
# - Adds new downloaded files.
#   (not listed in the FUG index HTML files)
# - Creates new HTML index files.
#   (Including the credits for the people providing
#    the file collections).
#
# Direct download link see $fug_url.
#
# This script can be executed on a UNIX/Linux environment or
# in Windows on mingw (e.g. git bash).
# The follwing tools have to be installed: sed, awk, wget.
#
# syntax:
#    download_fug_dsk_files.sh [-d] -D <target_dir>
# parameters:
#   -d:             Delete DSK files before downloading them
#   -D <target_dir> Download target directory

fug_url="http://www.flexusergroup.com/flexusergroup"
delete=no
index_prefix=fugd
target_dir=

# FUG subdirectories to be downloaded:
srcs="bbackstrom dfarnsworth fhl flex2 jlang ltaylor uk68mug windrush"
# Corresponding target download directories.
# index is a pseudo target used to create an index.html file.
tgts="index Bjarne_Baeckstroem Dan_Farnsworth Frank_Hogg_Labs FLEX2 Joe_Lang \
      Leo_Taylor UK_68_Micro_Group Windrush"
# Corresponding index html page (see also prefix $index_prefix):
# index is a pseudo target used to create an index.html file.
index_base="index bba dfa fhl flx2 jlan ltay ukug win"

usage() {
    echo "Syntax:"
    echo "    download_fug_dsk_files.sh [-h] [-d] -D <target_dir>"
    echo "Parameters:"
    echo "    -h:             Print this and exit."
    echo "    -d:             Delete DSK files before downloading them."
    echo "    -D <target_dir> Download target directory."
}

while [ "x$1" != "x" ]
do
    case "$1" in
        -h) usage; exit 0;;
        -d) delete=yes;;
        -D) shift; target_dir=$1;;
        --) shift; break;;
        *)  echo "*** Error: Invalid option '$1'."; usage; exit 1;;
    esac
    shift
done

if [ "x$target_dir" = "x" ]; then
    echo "*** Error: No target directory specified."
    usage
    exit 1
fi

wgetpath=`which wget 2>/dev/null`
if [ "x$wgetpath" = "x" ]; then
    echo "*** Error: wget not found."
    echo "  On a debian based system it can be installed with:"
    echo "  sudo apt-get install wget"
    exit 1
fi

# Option -d: delete previously downloaded files and intermediate directories
if [ -d $target_dir ]; then
    if [ "$delete" = "yes" ]; then
        rm -rf $target_dir
    fi
fi
if [ ! -d $target_dir ]; then
    mkdir $target_dir
fi

# Download the DSK file subdirectories as defined in $src.
set $tgts
shift # skip index pseudo target.
for src in $srcs
do
    sub_directory=${target_dir}/$1; shift
    if [ -d $sub_directory ]; then
        if [ "$delete" = "yes" ]; then
            rm -rf $sub_directory
        fi
    fi
    if [ ! -d $sub_directory ]; then
        echo "Download $sub_directory ..."
        mkdir $sub_directory

        wget_log=wget.log
        logfile=${sub_directory}/${wget_log}
        wget --recursive --no-parent --no-directories --no-host-directories \
             --tries=20 --wait=1 --random-wait --output-file=$logfile \
             --directory-prefix=$sub_directory ${fug_url}/DSKs/${src}/ \
        || echo "wget returned an error. Check log-files."

        rm -f ${sub_directory}/index.html >/dev/null 2>&1
        rm -f ${sub_directory}/index.html.1 >/dev/null 2>&1
        rm -f ${sub_directory}/index.html.2 >/dev/null 2>&1
    else
        echo "Skip $sub_directory (already downloaded)."
    fi
done

image_dir=images
fug_anim_file=fuganim.gif
if [ ! -d ${target_dir}/${image_dir} ]; then
    mkdir ${target_dir}/${image_dir}
fi
if [ ! -f ${target_dir}/${image_dir}/${fug_anim_file} ]; then
    echo "Download ${image_dir}/${fug_anim_file} ..."
    wget --quiet --no-directories --no-host-directories --tries=20 \
         --directory-prefix=$target_dir/${image_dir} ${fug_url}/${image_dir}/${fug_anim_file}
fi
fug_ico_file=fug.ico
if [ ! -f ${target_dir}/${image_dir}/${fug_ico_file} ]; then
    echo "Copy ${image_dir}/${fug_ico_file} ..."
    cp $(dirname $0)/${fug_ico_file} ${target_dir}/${image_dir}/${fug_ico_file}
fi

set $tgts
for src in $index_base
do
    target_content_file=${target_dir}/$1.html
    if [ -f $target_content_file ]; then
        echo "Skip $target_content_file creation (exists already)."
        shift 
        continue
    fi
    
    echo "Creating html index for $1 ..."
    if [ "$src" != "index" ]; then
        file=fugd${src}.htm
        if [ ! -f $target_dir/$file ]; then
            wget --quiet --no-directories --no-host-directories --tries=20 \
                 --directory-prefix=$target_dir ${fug_url}/${file}
        fi
        temp_file=/tmp/fug_data.tmp
        rm -f $temp_file >/dev/null 2>&1

        # Parse the FUG index files and store file name, contents and date
        # into a temp_file.
        # Print an error if a file can not be found in the downloaded files.
        # Fix upper/lower case mismatch.
        line=50
        last_line=`cat ${target_dir}/${file} | wc -l`
        while [ $line -le $last_line ]
        do
            dsk_file=`sed -n -e "$line s/.*\"DSKs[\\][A-Za-z0-9_\\]\+[\\]\([A-Za-z0-9_.-]\+\) \?\".*<td>\([!A-Za-z0-9_. <>():#/&\*'+-]\+\)<\/td><td>\([!A-Za-z0-9_. /<>-]\+\)<\/td>.*/\1/p" $target_dir/$file`
            contents=`sed -n -e "$line s/.*\"DSKs[\\][A-Za-z0-9_\\]\+[\\]\([A-Za-z0-9_.-]\+\) \?\".*<td>\([!A-Za-z0-9_. <>():#/&\*'+-]\+\)<\/td><td>\([!A-Za-z0-9_. /<>-]\+\)<\/td>.*/\2/p" $target_dir/$file | sed -e "s/<!-- insert Contents here -->//"`
            date=`sed -n -e "$line s/.*\"DSKs[\\][A-Za-z0-9_\\]\+[\\]\([A-Za-z0-9_.-]\+\) \?\".*<td>\([!A-Za-z0-9_. <>():#/&\*'+-]\+\)<\/td><td>\([!A-Za-z0-9_. /<>-]\+\)<\/td>.*/\3/p" $target_dir/$file | sed -e "s/<!-- insert Date here -->//"`

            if [ "x$dsk_file" != "x" ]; then
                result=`find ${target_dir}/$1 -name $dsk_file -print`
                if [ "x$result" != "x" ]; then
                    echo "$dsk_file|$contents|$date" >> $temp_file
                else
                    result=`find ${target_dir}/$1 -iname $dsk_file -print`
                    if [ "x$result" != "x" ]; then
                        # Fix upper/lower case mismatch
                        dsk_file=$(basename "$result")
                        echo "   Fixed upper/lower case mismatch to: $dsk_file" >&2
                        echo "$dsk_file|$contents|$date" >> $temp_file
                    else
                        echo "   Error: file $dsk_file not found" >&2
                    fi
                fi
            fi
            line=`expr $line + 1`
        done
        if [ "$1" = "Bjarne_Baeckstroem" ]; then
            collection="Bjarne Baeckstroem's Collection"
        else
            collection=`sed -n -e "s/.*<b>\([a-zA-Z0-9' -]\+Collection\).*/\1/p" ${target_dir}/${file}`
        fi
        thanksto=`sed -n -e "/.*\(With thanks.*\)\(<p>\|<br>\).*/p" ${target_dir}/${file}`
        rm -f ${target_dir}/${file} >/dev/null 2>&1
        # Iterate the downloaded files and add a file if it is not already
        # in temp_file. Ignore *.log and *.TXT files.
        contents="-"
        date="-"
        for dsk_file in `ls ${target_dir}/$1 -I *.log -I *.TXT`
        do
            result=`sed -n -e "s/\(${dsk_file}\).*/\1/p" $temp_file`
            if [ "x$result" = "x" ]; then
                echo "   Added file: $dsk_file" >&2
                echo "$dsk_file|$contents|$date" >> $temp_file
            fi
        done
    fi

    # Create a target index html file for the current target.
    echo "<!DOCTYPE html>" >> $target_content_file
    echo "<html>" >> $target_content_file
    echo "<head>" >> $target_content_file
    echo "<title>FLEX User Group - ${collection}</title>" >> $target_content_file
    echo "<meta name=\"description\" content=\"FLEX User Group - Historic Disk Collection\">" >> $target_content_file
    echo "<meta name=\"keywords\" content=\"FLEX, User Group, MC6809, DSK\">" >> $target_content_file
    echo "<meta name=\"author\" content=\"Wolfgang Schwotzer\">" >> $target_content_file
    echo "</head>" >> $target_content_file
    echo "<body text=\"#00FF00\" bgcolor=\"#000000\" link=\"#00E000\" vlink=\"#008000\">" >> $target_content_file
    echo "<table border=0>" >> $target_content_file
    echo "<tr>" >> $target_content_file
    echo " <td><img src=\"${image_dir}/${fug_anim_file}\" height=60 width=92 align=top></td>" >> $target_content_file
    echo " <td>" >> $target_content_file
    echo " <h1>FLEX User Group - Historic Disk Collection</h1>" >> $target_content_file
    echo " </td>" >> $target_content_file
    echo "</tr>" >> $target_content_file
    echo "</table>" >> $target_content_file
    echo "<table border=0 width=640>" >> $target_content_file
    echo "<tr>" >> $target_content_file
    echo " <td valign=\"top\" width=280>" >> $target_content_file
    echo " <b>Collections available:</b><br><br>" >> $target_content_file

    # Create a link list to the other file Collections:
    for tgt in $tgts
    do
        if [ "${tgt}" != "$1" ]; then
            tgt_space=`echo $tgt | sed -e "s/_/ /g"`
            if [ "${tgt}" = "index" ]; then
                tgt_space="Home"
            fi
            echo " <a href=\"${tgt}.html\">${tgt_space}</a><br>" >> $target_content_file
        fi
    done
    
    echo " </td>" >> $target_content_file
    echo " <td valign=\"top\" width=800>" >> $target_content_file

    if [ "$src" != "index" ]; then
        # Add the table with the files (file name, contents, date)
        file_count=`cat ${temp_file} | wc -l`
        files="$file_count file"
        if [ "$file_count" != "1" ]; then
            files="${files}s"
        fi
        echo " <b>$collection ($files)</b><br>" >> $target_content_file
        echo " $thanksto<p>" >> $target_content_file

        echo "<table>" >> $target_content_file
        echo "<tr><th>File Name</th><th>Contents</th><th>Date</th></tr>" >> $target_content_file
        awk "{ printf(\" <tr><td><a href=\\\"$1/%s\\\">%s</a></td><td>%s</td><td>%s</td></tr>\\n\", \$1, \$1, \$2, \$3) }" FS=\| $temp_file >> $target_content_file
        echo "</table>" >> $target_content_file
    fi

    echo "</td>" >> $target_content_file
    echo "</tr>" >> $target_content_file
    echo "</table>" >> $target_content_file
    echo "</body>" >> $target_content_file
    echo "</html>" >> $target_content_file
    rm -f $temp_file >/dev/null 2>&1

    shift
done

