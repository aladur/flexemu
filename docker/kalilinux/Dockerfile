# When using the default kali download server (http://http.kali.org/) a mirror
# nearby is chosen. If this mirror is http://mirror.johnnybegood.fr/kali/
# there are certificate issues. For this reason a reliable hard coded mirror
# is set in /etc/apt/sources.list

FROM kalilinux/kali-rolling:latest

RUN echo "deb http://mirror.netcologne.de/kali kali-rolling main contrib non-free non-free-firmware" > /etc/apt/sources.list && \
    apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev && \
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

