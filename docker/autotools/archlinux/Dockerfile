# Search for archlinux packages:
# https://archlinux.org/packages/

FROM archlinux:base-devel

ARG GIT_COMMIT
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
     ncurses \
     qt5-base \
     qt5-x11extras \
     ttf-dejavu-nerd \
     gnu-free-fonts && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

