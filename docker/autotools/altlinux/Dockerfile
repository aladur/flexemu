# Search for ALT Linux packages: https://packages.altlinux.org/en/search/

FROM alt:sisyphus

RUN apt-get -y update && apt-get -y install \
     sudo \
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
     fonts-ttf-dejavu && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
    usermod --append --groups wheel user
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

