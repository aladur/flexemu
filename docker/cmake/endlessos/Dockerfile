FROM endlessm/eos:6.0

ARG GIT_COMMIT
RUN apt-get -y update && apt-get -y install \
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
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

