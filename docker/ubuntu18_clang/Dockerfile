# Create user "ubuntu".
# It is used and prepared for full sudo access without password.
# use clang-7 compiler and libstdc++-8 c++ library.
# The filesystem library is a separate library to be linked with
# LIBS=-lstdc++fs

FROM asappinc/ubuntu18:18.04.0

RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     libstdc++-8-dev \
     clang-7 \
     libncurses-dev \
     libncursesw5 \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev && \
    useradd -m -d /home/username -s /bin/bash ubuntu && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
ENV QT_XCB_GL_INTEGRATION=none
USER ubuntu
WORKDIR /home/ubuntu
ENV QT_SELECT=qt5
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc CC=clang-7 CXX=clang++-7 && \
    make -j8 check LIBS=-lstdc++fs && \
    sudo make install
CMD ["/bin/bash"]

