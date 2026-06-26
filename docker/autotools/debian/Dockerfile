FROM debian:12.6-slim

RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev && \
    useradd --create-home --shell /bin/bash user && \
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

