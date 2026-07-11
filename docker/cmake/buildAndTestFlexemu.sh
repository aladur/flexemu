#!/bin/sh
#
# parameters:
#  $1         (optional) The git commit hash or tag to checkout. If not set
#             master is used.

git clone https://github.com/aladur/flexemu.git
cd flexemu
if [ -n "$1" ]; then
    git checkout "$1"
fi
git submodule update --init --recursive
cmake -S . -B ../flexemu_build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc
cd ../flexemu_build
cmake --build . -j8
ctest -V
sudo cmake --install .
