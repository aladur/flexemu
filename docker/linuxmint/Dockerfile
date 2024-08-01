FROM linuxmintd/mint21.3-amd64

WORKDIR /home/user
RUN apt-get -y update && apt-get install -y \
    xxd \
    vim \
    git \
    build-essential \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5x11extras5-dev
RUN git clone https://github.com/aladur/flexemu.git && \
    cd /home/user/flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

