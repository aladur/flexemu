#!/bin/sh
#
# parameters:
#  $1         (required) The git commit hash or tag to checkout

git clone https://github.com/aladur/flexemu.git
if [ ! -n "$1" ]; then
    echo "**** Error: A git commit hash or tag has to be specified."
    echo "     Use: --build-arg GIT_COMMIT=<git_commit>"
    echo "     For example: --build-arg GIT_COMMIT=V3.31"
    exit 1
fi
cd flexemu
git checkout "$1"
git submodule update --init --recursive
./configure --prefix=/usr --sysconfdir=/etc
make -j8 check
sudo make install
