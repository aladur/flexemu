# Search for ALT Linux packages: https://packages.altlinux.org/en/search/
FROM alt:sisyphus

RUN apt-get -y update && apt-get -y install \
    xxd \
    vim-console \
    git \
    build-essential \
    libqt5-core \
    libqt5-gui \
    libqt5-widgets \
    libqt5-printsupport \
    qt5-x11extras-devel \
    libqt5-x11extras \
    fonts-ttf-dejavu
ENV QT_XCB_GL_INTEGRATION=none
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

