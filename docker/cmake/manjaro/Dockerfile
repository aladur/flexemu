# manjaro linux has already a user "builder" with uid 1000 defined.
# available. It is used and prepared for full sudo access without password.

FROM manjarolinux/base:latest

ARG GIT_COMMIT
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm  -y \
     sudo \
     xxd \
     vim \
     git \
     which \
     make \
     cmake \
     gcc \
     ncurses \
     qt5-base \
     qt5-x11extras \
     ttf-dejavu-nerd \
     gnu-free-fonts && \
     echo "builder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER builder
WORKDIR /home/builder
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

