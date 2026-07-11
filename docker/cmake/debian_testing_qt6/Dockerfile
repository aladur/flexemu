FROM debian:testing-slim

ARG GIT_COMMIT
RUN apt-get -y update && apt-get -y install \
    sudo \
    xxd \
    vim \
    git \
    build-essential \
    cmake \
    libncurses-dev \
    qt6-base-dev \
    qt6-base-dev-tools && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestQt6Flexemu.sh /tmp/buildAndTestQt6Flexemu.sh
ENV QT_SELECT=qt6
ENV PATH=/usr/lib/qt6/bin:/usr/lib/qt6/libexec:$PATH
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestQt6Flexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

