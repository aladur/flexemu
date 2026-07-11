FROM alpine:3.24.1

ARG GIT_COMMIT
RUN apk update && apk upgrade && \
    apk add \
     sudo \
     xxd \
     vim \
     git \
     make \
     cmake \
     build-base \
     g++ \
     ncurses-dev \
     qt5-qtbase-dev \
     qt5-qtx11extras-dev \
     mesa-utils \
     font-freefont && \
    adduser --disabled-password --shell /bin/ash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

