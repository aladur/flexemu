FROM photon:5.0

RUN tdnf update -y && \
    tdnf upgrade -y && \
    tdnf install -y \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
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

