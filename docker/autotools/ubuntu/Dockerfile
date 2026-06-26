# ubuntu linux has already a user "ubuntu" with uid 1000 defined.
# available. It is used and prepared for full sudo access without password.

FROM ubuntu:24.04

RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_GL_INTEGRATION=none
USER ubuntu
WORKDIR /home/ubuntu
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

