FROM debian:testing-slim

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
ENV QT_SELECT=qt6
ENV PATH=/usr/lib/qt6/libexec:$PATH
ENV PATH=/usr/lib/qt6/bin:$PATH
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

