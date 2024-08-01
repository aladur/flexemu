FROM --platform=linux/s390x s390x/debian:12.6-slim

ENV QT_XCB_FORCE_SOFTWARE_OPENGL=1
RUN apt-get -y update && apt-get install -y \
    git \
    xxd \
    vim \
    build-essential \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5x11extras5-dev \
    libglvnd-dev \
    mesa-utils
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

