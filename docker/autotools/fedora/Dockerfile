FROM fedora:40

ARG GIT_COMMIT
RUN dnf upgrade -y && \
    dnf install -y \
     sudo \
     xxd \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
     gcc-g++ \
     ncurses-devel \
     qt5-qtbase-devel \
     qt5-qtx11extras-devel \
     liberation-mono-fonts && \
     useradd --create-home --shell /bin/bash user && \
     echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

