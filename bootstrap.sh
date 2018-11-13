#!/bin/bash

apt-get update
apt-get dist-upgrade -y
apt-get install -y build-essential git libftdi-dev libftdi1 doxygen python3-pip libsdl2-dev curl cmake libusb-1.0-0-dev
ln -s /usr/bin/libftdi-config /usr/bin/libftdi1-config
curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash
apt-get install git-lfs
git lfs install
pip3 install pyelftools
usermod -a -G dialout vagrant

touch 90-ftdi_gapuino.rules
echo 'ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6010", MODE="0666", GROUP="dialout"'> 90-ftdi_gapuino.rules
mv 90-ftdi_gapuino.rules /etc/udev/rules.d/
udevadm control --reload-rules && udevadm trigger
git clone https://github.com/GreenWaves-Technologies/gap_sdk.git /opt/gap_sdk
git lfs clone https://github.com/GreenWaves-Technologies/gap_riscv_toolchain.git /usr/lib/gap_riscv_toolchain

cd /usr/lib/gap_riscv_toolchain/bin
for eachFile in *
do
    ln -fs $PWD/$eachFile /usr/bin/$eachFile
done

cd /opt/gap_sdk
git submodule update --init --recursive

source sourceme.sh
make all

echo "source /opt/gap_sdk/sourceme.sh" >> /home/vagrant/.bashrc

echo "Done! :)"
