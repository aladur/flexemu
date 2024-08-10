# Look for mageia packages: https://rpmfind.net
# (https://madb.mageia.org is very slow)
# A small hack for libQt5Network.so is needed so that it can be found
# when detecting for qt components.
# An extenstion to the PATH variable is needed to detect moc, uic and rcc.

FROM mageia:9

RUN dnf upgrade -y && \
    dnf install -y \
     vim-enhanced \
     git \
     make \
     automake \
     autoconf \
     gcc \
     gcc-c++ \
     qtbase5-common-devel \
     libqt5core-devel \
     libqt5gui-devel \
     libqt5widgets-devel \
     libqt5printsupport-devel \
     libqt5x11extras-devel \
     libqt5opengl-devel \
     libqt5concurrent-devel \
     libqt5test-devel \
     libqt5sql-devel \
     libqt5xml-devel \
     fonts-ttf-dejavu
ENV QT_XCB_GL_INTEGRATION=none
RUN mkdir -p /home/user && \
    cd /usr/lib64 && \
    ln -s libQt5Network.so.5.15.7 libQt5Network.so && \
    export PATH=$PATH:/usr/lib64/qt5/bin && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

