# Search for ALT Linux packages: https://packages.altlinux.org/en/search/

FROM alt:sisyphus

ARG GIT_COMMIT
RUN apt-get -y update && apt-get -y install \
     sudo \
     xxd \
     vim-console \
     git \
     build-essential \
     libncursesw-devel \
     libqt5-core \
     libqt5-gui \
     libqt5-widgets \
     libqt5-printsupport \
     qt5-x11extras-devel \
     libqt5-x11extras \
     fonts-ttf-dejavu && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
    usermod --append --groups wheel user

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

