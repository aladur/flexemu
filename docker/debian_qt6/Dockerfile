FROM debian:12.6-slim

RUN apt-get -y update && apt-get -y install \
    xxd \
    vim \
    git \
    build-essential \
    qt6-base-dev \
    qt6-base-dev-tools
ENV PATH=/usr/lib/qt6/libexec:$PATH
ENV PATH=/usr/lib/qt6/bin:$PATH
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

