FROM --platform=linux/s390x s390x/debian:12.6-slim

RUN apt-get -y update && apt-get install -y \
     sudo \
     git \
     xxd \
     vim \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev \
     libglvnd-dev \
     mesa-utils && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_FORCE_SOFTWARE_OPENGL=1
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

