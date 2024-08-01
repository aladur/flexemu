FROM debian:testing-slim

RUN apt-get -y update && apt-get -y install \
    xxd \
    vim \
    git \
    build-essential \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5x11extras5-dev
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

