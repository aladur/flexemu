# docker build failed. Photon OS does not provide qt5 packages.
# Search for packages: yum search <package-query>

FROM photon:5.0

ARG GIT_COMMIT
RUN tdnf update -y && \
    tdnf upgrade -y && \
    tdnf install -y \
     sudo \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
     qt5-qtbase-devel \
     qt5-qtx11extras-devel && \
    useradd --create-home --shell /bin/bash user && \
    echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

