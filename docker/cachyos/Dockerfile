# Search for cachyos/archlinux packages:
# https://archlinux.org/packages/

FROM cachyos/cachyos-v3

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm  -y \
     sudo \
     xxd \
     vim \
     git \
     which \
     make \
     automake \
     autoconf \
     gcc \
     qt5-base \
     qt5-x11extras \
     ttf-dejavu-nerd \
     gnu-free-fonts && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

