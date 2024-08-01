#!/usr/bin/bash
#
# Run an executable with a graphical user interface (GUI) based on
# the X11 protocol using docker. The command line parameters used for
# the docker run command forward X11 to the host.
# The image may contain any kind of CPU archicture, it is detected
# automatically. Architectures different to the host architecture
# are emulated using qemu as backend.

USAGE=`cat << EOF
Usage: dockerRunForwardX11.sh <docker-image> <executable> [<param>...]

Parameters:
       <docker-image> The docker image id or name.
       <executable>   The executable within the docker image.
       <param>        Command line parameter for <executable>.

EOF`

if [ $# -lt 2 ]; then
  printf "%s\n" "$USAGE"
  exit 1
fi

host_arch=amd64
client_arch=`docker image inspect -f "{{json .Architecture }}" $1 | \
      sed -n "s/\"\([a-z0-9]\+\)\"/\1/p"`

if [ "$client_arch" != "$host_arch" ]; then
# run docker image with a different CPU architecture.
# Different CPU architectures are emulated wit qemu backend. For this case
# only OpenGL software rendering works. The following environment variables
# LIBGL_ALWAYS_SOFTWARE and QT_XCB_NO_MITSHM force a OpenGL software rendering.
# Before run docker for a different CPU architecture this installation has
# to be done once:
#
#    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
#
    docker run -it --rm \
        --platform linux/$client_arch \
        --network host \
        -e LIBGL_ALWAYS_SOFTWARE=1 \
        -e QT_XCB_NO_MITSHM=1 \
        -e DISPLAY=${DISPLAY:-:0.0} \
        -h $HOSTNAME \
        -v /tmp/.X11-unix/:/tmp/.X11-unix \
        -v $XAUTHORITY:/root/.Xauthority:rw \
        $1 $2 $3 $4 $5 $6 $7 $8 $9
else
    docker run -it --rm \
        --network host \
        -e DISPLAY=${DISPLAY:-:0.0} \
        -h $HOSTNAME \
        -v /tmp/.X11-unix/:/tmp/.X11-unix \
        -v $XAUTHORITY:/root/.Xauthority:rw \
        -v /dev/dri/:/dev/dri/ \
        $1 $2 $3 $4 $5 $6 $7 $8 $9
fi

