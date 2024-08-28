FROM debian:unstable-slim

RUN apt-get -y update && apt-get -y install \
    sudo \
    xxd \
    vim \
    git \
    build-essential \
    qt6-base-dev \
    qt6-base-dev-tools && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV PATH=/usr/lib/qt6/libexec:$PATH
ENV PATH=/usr/lib/qt6/bin:$PATH
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

