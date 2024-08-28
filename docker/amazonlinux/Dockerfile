# BUILD FAILURE:
# Amazonlinux 2023 does not support qt5
# (and amazonlinux 2 is too old resulting in build failures).
# 2023 Package list:
# https://docs.aws.amazon.com/linux/al2023/release-notes/all-packages-AL2023.5.html

FROM amazonlinux:2023

RUN yum update -y && \
    yum upgrade -y && \
    yum install -y \
     sudo \
     xxd \
     vim \
     git \
     tar \
     gcc \
     gcc-c++ \
     automake \
     autoconf \
     qt5-qtbase-devel \
     qt5-qtx11extras-devel && \
     useradd --create-home --shell /bin/bash user && \
     echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
RUN curl https://ftp.gnu.org/gnu/make/make-4.4.tar.gz >make-4.4.tar.gz
    tar xzf make-4.4.tar.gz
    cd make-4.4
    ./configure
    make -j8
    sudo make install
    export PATH=/usr/local/bin:$PATH
    cd ..
    git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

