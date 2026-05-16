FROM debian:trixie-slim

RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     build-essential \
     libxaw7-dev \
     libwxgtk3.2-dev && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git checkout v2.15 && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install && \
    sudo mkdir -p /etc/X11/app-defaults && \
    sudo cp src/Flexemu.ad /etc/X11/app-defaults/Flexemu
CMD ["/bin/bash"]

