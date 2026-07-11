FROM oraclelinux:10-slim

ARG GIT_COMMIT
RUN microdnf install -y dnf
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
     qt6-qtbase-devel \
     liberation-mono-fonts && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestQt6Flexemu.sh /tmp/buildAndTestQt6Flexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

