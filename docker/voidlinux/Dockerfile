# Search for void linux packages:
# https://voidlinux.org/packages/

FROM voidlinux/voidlinux
# Also works based on this voidlinux docker image:
# FROM privb0x23/void:glibc

# Remarks:
# - Use a different repository URL. The one referenced in the docker image
#   does not work.
# - A second sync (-S) is needed after updating xbps. Otherwise no package
#   is found.
# - ERROR: base-files-0.144_1: [unpack] failed to unpack files from archive:
#   Invalid argument
#   The reason for this is that during installation of base-files /etc/hosts
#   is touched but in the docker image it is read-only.
#   Solution: Add a noextract policy for file /etc/hosts

RUN echo repository=https://repo-default.voidlinux.org/current/ > /usr/share/xbps.d/00-repository-main.conf
RUN echo noextract=/etc/hosts > /usr/share/xbps.d/01-noextract-policy.conf
RUN xbps-install -Sy && \
     xbps-install -uy xbps ca-certificates && \
     xbps-install -Sy && \
     xbps-install -uy && \
     xbps-query -L && \
     xbps-remove -oOy && \
     xbps-install -y \
     sudo \
     bash \
     xxd \
     vim \
     git \
     make \
     automake \
     autoconf \
     gcc \
     qt5-core \
     qt5-gui \
     qt5-widgets \
     qt5-printsupport \
     qt5-devel \
     qt5-x11extras-devel \
     dejavu-fonts-ttf && \
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

