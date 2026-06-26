# manjaro linux has already a user "builder" with uid 1000 defined.
# available. It is used and prepared for full sudo access without password.

FROM manjarolinux/base:latest

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
     echo "builder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_GL_INTEGRATION=none
USER builder
WORKDIR /home/builder
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

