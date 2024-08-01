FROM manjarolinux/base:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm  -y \
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
     gnu-free-fonts
ENV QT_XCB_GL_INTEGRATION=none
RUN mkdir -p /home/user && \
    cd /home/user && \
    git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]

