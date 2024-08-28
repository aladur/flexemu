FROM alpine:3.20.1

RUN apk update && apk upgrade && \
    apk add \
     sudo \
     xxd \
     vim \
     git \
     make \
     automake \
     autoconf \
     build-base \
     g++ \
     qt5-qtbase-dev \
     qt5-qtx11extras-dev \
     mesa-utils \
     font-freefont && \
    adduser --disabled-password --shell /bin/ash user && \
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

