# Use docker to run and test flexemu

Docker provides a lightweight and resource efficient virtualization. It can be used to quickly run and test flexemu project on different linux distributions.

To use docker on linux it first has to be installed. The installation steps are [documented here](https://docs.docker.com/desktop/) under the topic "Install docker Desktop". It can be installed on Windows, MacOS or linux. This HowTo focusses on linux host. So the one or the other step may be different on a MacOS or Windows host.

## Build a docker image

The docker build files in this project come in two flavours. The first is `autotools`, it allows to build this project using `autoconf` and `automake`. The second is `cmake`. it allows to build this project using `cmake`. Both flavours are represented in corresponding subfolders.

Within `autotools` or `cmake` another folder hierarchy represents the linux distribution. Each subfolder contains a Dockerfile which can be used to create a docker image for a specific linux distribution, sometimes with variants like building with Qt5 or Qt6, or with gcc or clang. All Dockerfiles contain the same steps. Depending on the used docker base image the one or the other step is skipped because the necessary dependeny is already available in the used docker base image. For example the debian base image already contains some fonts so this step can be skipped. These are the steps executed in the Dockerfile.

* Update and upgrade package manger
* Install development environment
* Install Qt libraries and development headers
* Install some fonts
* Create a build user (with default access rights and sudo capability)
* Switch to build user
* Download flexemu distribution from github
* Checkout a specific git commit or tag
* Initialize and update git submodules
* Configure flexemu project
* Build flexemu and execute unittests
* Install flexemu (with root access)

Docker provides an easy to use command line tool to build a docker image.

        docker build --build-arg GIT_COMMIT=<commit> -f <distribution>/Dockerfile -t <name>:<tag> .

Explicitly specifying the path to the Dockerfile by `-f <distribution>/Dockerfile` allows to widen the context and for example use a reusable shell script in the current directory.

Please do not forget the dot at the end of `docker build` command. To avoid confusion about the linux distribution I introduced a naming convention for the docker image to use `<distribution>_flexemu` for `<name>` which is used within this HowTo. `<tag>` either can be `latest` or a git tag. Other naming conventions can make sense to maybe also take the version of the linux distribution into account, so this Howto simply is ment as a guide line.

There is a parameter `--build-arg GIT_COMMIT=<git_commit>` to specify a git commit hash or tag to checkout for the build.

For example flexemu on based on a [Alpine linux docker base image](https://hub.docker.com/_/alpine) using git tag `V3.32.0` can be built as follows.

        docker build --build-arg GIT_COMMIT=V3.32.0 -f alpine/Dockerfile -t alpine_flexemu:3.32.0 .

The build takes several minutes and after this the docker image can be referenced by it's name `alpine_flexemu:3.32.0`.

When building with `autotools` the argument `GIT_COMMIT` always has to be specified. Only versions up to including V3.31 are supported.

When building with `cmake` the argument `GIT_COMMIT` is optional. Only versions V3.30 or higher are supported. If not specified the current `master` is build.

## Run a docker image

Docker also provides an easy to use command line tool to run a docker image.

        docker run -it <distribution>_flexemu:<tag> <executable> [<param>...]

This can be used for all flexemu command line tools like
`flex2hex`,
`hex2flex`,
`dsktool`,
`mdcrtool`,
`bin2s19`,
`fromflex` and
`toflex`.

For example to print help for `dsktool` the following command line can be used:

        docker run -it alpine_flexemu:latest dsktool -h

By default a docker image does not store changes made in the file system. Instead a directory (`-v` for volume) can be set within docker to exchange files. The parameter syntax is `-v <local-dir>:<docker-dir>`. If any of the directories do not exist yet they are created with root access.

For example to create a new disk image file with  `dsktool` the following command line can be used:

        docker run -v ~/exchange:/home/exchange -it alpine_flexemu:latest dsktool -f /home/exchange/new.dsk -S80dsdd

This creates a file `new.dsk` in local directory `~/exchange` (on the host).

## Run a docker image with X11 forwarding

`flexemu` and `flexplorer` provide a graphical user interface (GUI). On the most linux systems this is still based on X11 protocol. Docker provides the possiblity to forward the X11 protocol form the docker engine to the host. This way the executable runs within the docker engine and is displayed on the X-Server of the host. To hide the necessary command line parameters from the user there is a helper shell script available, `dockerRunForwardX11.sh` to automatically forward X11. For example to run flexemu within Alpine linux can be easily done with the following command line:

        dockerRunForwardX11.sh alpine_flexemu:latest flexemu

The generic syntax of this command is:

        dockerRunForwardX11.sh <docker-image-or-id> <executable> [<param>...]

To call this shell script from anywhere it can be copied into a folder which is part of the `PATH` environment variable.

## Big endian support

amd64 CPU architecture is a *little endian* CPU. That means that for the following C variable definition

        uint32_t var = 1U;

the byte content of `var` is `0x01, 0x00, 0x00, 0x00`, means the least significant 8-bit are stored in the first byte. There are also *big endian* CPU architectures available. They would store the most significant 8-bit in the first byte. In some points flexemu depends on the endianess of a CPU. See macro `WORDS_BIGENDIAN` for details. Also bitfields may be aligned as little or big endian. See macro `BITFIELDS_LSB_FIRST` for details.

Docker also supports virtualizing CPU architectures different from the host CPU architectures using qemu as backend.

The IBM s390x CPU is a big endian CPU architcture. For both byte ordering and bitfield ordering it uses *big endian* format. There are [docker debian base images for s390x architecture](https://hub.docker.com/r/s390x/debian/) available.

### Build a docker image

As a first step the `qemu` backend has to be initialized once within a session with the following command line.

        docker run --rm --privileged multiarch/qemu-user-static --reset -p yes --credential yes

With the following command line a debian docker image with s390x CPU architecture can be build. To build on a different architecture the Dockerfile contains a parameter `--platform=<os/cpu_architecture>`, in this case specific for the s390 CPU `--platform=linux/s390x` in the `FROM` command. This automatically activates qemu backend. The Dockerfile to build a debian image for the s390x CPU architecture is located in folder `debian_be`, the `_be` suffix means *big endian*.

        docker build -f debian_be/Dockerfile -t debian_be_flexemu:latest .

This build process takes significantly more time due to software emulation of the s390s CPU.

### Run a docker image

Running such a docker image there is no difference in the command line parameters independent of the virtualized CPU architecture.

So the following command line executes any flexemu command line tool running within debin on s390x.

        docker run --platform=linux/s390x -it debian_be_flexemu:latest <executable> [<param>...]

For running `flexemu` or `flexplorer` there is also no difference in the command line.

        dockerRunForwardX11.sh debian_be_flexemu:latest <executable> [<param>...]

The script automatically cares about the `--platform=linux/s390x` parameter.

