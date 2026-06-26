# Query packages:
# zypper search -s <package-query>

FROM opensuse/leap:15.6

RUN zypper --non-interactive update && \
    zypper --non-interactive install --type pattern devel_basis && \
    zypper --non-interactive install \
    sudo \
    vim \
    git \
    gcc11 \
    gcc11-c++ \
    libqt5-qtbase-devel \
    libqt5-qtx11extras-devel \
    dejavu-fonts && \
   useradd --create-home --shell /bin/bash user && \
   echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    CC=gcc-11 CXX=g++-11 ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    sudo make install
CMD ["/bin/bash"]

