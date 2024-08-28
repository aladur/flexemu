FROM aclemons/slackware:15.0

WORKDIR /tmp
COPY setup.sh setup.sh
SHELL ["/usr/bin/bash", "-c"]
RUN ./setup.sh
ENV GIT_SSL_NO_VERIFY=true
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
