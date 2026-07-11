#!/usr/bin/bash

# Execute slackpkg and intentionally ignore the exit code.
# Good site to look for slackware packages: https://pkgs.org or
# https://packages.slackware.com/
# All dependencies have been resolved by hand :-(

useradd --create-home --shell /bin/bash user
echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
slackpkg -batch=on -default_answer=y update || true
slackpkg -batch=on -default_answer=y upgrade-all || true
slackpkg -batch=on -default_answer=y clean-system || true
slackpkg -batch=on -default_answer=y install \
    kernel-headers \
    sudo \
    libglvnd \
    libXau \
    libXext \
    libXdmcp \
    libxcb \
    libX11 \
    libXcomposite \
    libXrender \
    libxml2 \
    libxslt \
    xorgproto \
    lz4 \
    sqlite \
    orc \
    opus \
    libvorbis \
    libogg \
    flac \
    libsndfile \
    libasyncns \
    libarchive \
    elfutils \
    gstreamer \
    gst-plugins-base \
    pulseaudio \
    nghttp2 \
    brotli \
    dbus \
    icu4c \
    glibc \
    glib2 \
    guile \
    gc \
    woff2 \
    libwebp \
    hyphen \
    freetype \
    freetype \
    fontconfig \
    xcb-proto \
    xcb-util \
    xcb-util-cursor \
    xcb-util-image \
    xcb-util-keysyms \
    xcb-util-renderutil \
    xcb-util-wm \
    libSM \
    libICE \
    libxkbcommon \
    harfbuzz \
    graphite2 \
    dejavu-fonts-ttf \
    cyrus-sasl \
    ncurses \
    pkg-config \
    vim \
    git \
    flex \
    gcc \
    binutils \
    make \
    cmake \
    qt5

