# As derivative of ubuntu there is already a user "ubuntu" with uid 1000
# defined. It is used and prepared for full sudo access without password.

FROM linuxmintd/mint22.3-amd64

ARG GIT_COMMIT
RUN apt-get -y update && apt-get install -y \
    sudo \
    xxd \
    vim \
    git \
    build-essential \
    cmake \
    libncurses-dev \
    qtbase5-dev \
    qtbase5-dev-tools \
    libqt5x11extras5-dev && \
    echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER ubuntu
WORKDIR /home/ubuntu
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

