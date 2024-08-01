FROM aclemons/slackware:15.0

WORKDIR /home/user
COPY setup.sh setup.sh
SHELL ["/usr/bin/bash", "-c"]
RUN ./setup.sh
ENV GIT_SSL_NO_VERIFY=true
ENV QT_XCB_GL_INTEGRATION=none
RUN git clone https://github.com/aladur/flexemu.git && \
    cd flexemu && \
    git submodule update --init --recursive && \
    ./configure --prefix=/usr --sysconfdir=/etc && \
    make -j8 check && \
    make install
CMD ["/bin/bash"]
