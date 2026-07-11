# Query packages:
# zypper search -s <package-query>

FROM opensuse/leap:16.0

ARG GIT_COMMIT
RUN zypper --non-interactive update && \
    zypper --non-interactive install --type pattern devel_basis && \
    zypper --non-interactive install \
    sudo \
    vim \
    git \
    cmake \
    gcc-15 \
    gcc-c++-15 \
    libncurses6 \
    libqt5-qtbase-devel \
    libqt5-qtx11extras-devel \
    dejavu-fonts && \
   useradd --create-home --shell /bin/bash user && \
   echo "user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]

