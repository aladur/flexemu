# Look for almalinux packages:
# https://pkgs.org
# almalinux 10.2 only has qt6 packages

FROM almalinux:10.2

ARG GIT_COMMIT
RUN dnf upgrade -y && \
    dnf install -y \
     sudo \
     vim \
     git \
     make \
     cmake \
     gcc \
     gcc-g++ \
     ncurses-devel \
     cups-devel \
     qt6-qtbase-devel \
     liberation-mono-fonts && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestQt6Flexemu.sh /tmp/buildAndTestQt6Flexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestQt6Flexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

