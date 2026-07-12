FROM corpusops/ubuntu:20.04

ARG GIT_COMMIT
RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim \
     git \
     build-essential \
     cmake \
     libncurses-dev \
     build-essential \
     qtbase5-dev \
     qtbase5-dev-tools \
     libqt5x11extras5-dev && \
    useradd -m -d /home/username -s /bin/bash ubuntu && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER ubuntu
WORKDIR /home/ubuntu
ENV QT_SELECT=qt5
RUN mkdir -p /tmp/runtime-ubuntu && \
    /tmp/buildAndTestFlexemu.sh
CMD ["/bin/bash"]

