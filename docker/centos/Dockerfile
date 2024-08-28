# Search for centos packages: https://pkgs.org

FROM dokken/centos-stream-9:latest

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
     qt5-qtbase-devel \
     qt5-qtx11extras-devel \
     liberation-mono-fonts && \
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

