FROM fedora:40

RUN dnf upgrade -y && \
    dnf install -y \
     xxd \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
     gcc-g++ \
     qt5-qtbase-devel \
     qt5-qtx11extras-devel
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

