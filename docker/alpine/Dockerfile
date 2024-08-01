FROM alpine:3.20.1

WORKDIR /home/user
RUN apk update && apk upgrade && \
    apk add \
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
     font-freefont
RUN git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

