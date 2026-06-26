# docker build failed. Photon OS does not provide qt5 packages.
# Search for packages: yum search <package-query>

FROM photon:5.0

RUN tdnf update -y && \
    tdnf upgrade -y && \
    tdnf install -y \
     sudo \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
     qt5-qtbase-devel \
     qt5-qtx11extras-devel && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

