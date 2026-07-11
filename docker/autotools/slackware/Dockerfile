FROM aclemons/slackware:15.0

ARG GIT_COMMIT
WORKDIR /tmp
COPY slackware/setup.sh setup.sh
SHELL ["/usr/bin/bash", "-c"]
RUN ./setup.sh
ENV GIT_SSL_NO_VERIFY=true
ENV QT_XCB_GL_INTEGRATION=none
USER user
WORKDIR /home/user
COPY ../buildAndTestFlexemu.sh /tmp/buildAndTestFlexemu.sh
RUN /tmp/buildAndTestFlexemu.sh $GIT_COMMIT
CMD ["/bin/bash"]
