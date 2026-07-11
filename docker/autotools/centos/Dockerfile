# Search for centos packages: https://pkgs.org

FROM dokken/centos-stream-9:latest

ARG GIT_COMMIT
RUN yum update -y && \
    yum upgrade -y && \
    yum install -y \
     sudo \
     vim \
     git \
     gcc \
     gcc-c++ \
     automake \
     autoconf \
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

