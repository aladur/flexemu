FROM opensuse/leap:15.6

RUN zypper --non-interactive update && \
    zypper --non-interactive install --type pattern devel_basis && \
    zypper --non-interactive install \
    vim \
    git \
    gcc11 \
    gcc11-c++ \
    libqt5-qtbase-devel \
    libqt5-qtx11extras-devel \
    xorg-x11-fonts
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    CC=gcc-11 CXX=g++-11 ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

