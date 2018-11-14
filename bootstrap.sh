#!/bin/bash

# Basic bootstrap file to set up a Vagrant box to GAP8/Gapuino environment.

# Update, upgrade and install dependencies
apt-get update && apt-get dist-upgrade -y
apt-get install -y build-essential git libftdi-dev libftdi1 doxygen\
                   python3-pip libsdl2-dev curl cmake libusb-1.0-0-dev
curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash
apt-get install git-lfs
git lfs install
pip3 install pyelftools

# Adding vagrant user to dialout group for USB access
usermod -a -G dialout vagrant

# FTDI (USB) access rules
ln -s /usr/bin/libftdi-config /usr/bin/libftdi1-config
touch 90-ftdi_gapuino.rules
echo 'ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6010", MODE="0666", GROUP="dialout"'> 90-ftdi_gapuino.rules
mv 90-ftdi_gapuino.rules /etc/udev/rules.d/
udevadm control --reload-rules && udevadm trigger

# Cloning into gap_sdk and precompiled RISC-V Toolchain
git clone https://github.com/GreenWaves-Technologies/gap_sdk.git /opt/gap_sdk
git lfs clone https://github.com/GreenWaves-Technologies/gap_riscv_toolchain.git /usr/lib/gap_riscv_toolchain

# Linking Toolchain binaries to system folder
cd /usr/lib/gap_riscv_toolchain/bin
for eachFile in *
do
    ln -fs $PWD/$eachFile /usr/bin/$eachFile
done

# Loading git submodules sourcing the env and compiling everything
cd /opt/gap_sdk
git submodule update --init --recursive
source sourceme.sh
make all

# .bashrc settings
# Source to add gap_sdk env
echo "source /opt/gap_sdk/sourceme.sh" >> /home/vagrant/.bashrc
# Go to vagrant folder when login
echo "cd /vagrant" >> /home/vagrant/.bashrc
# Alias to make cd go to vagrant folder by default
echo "alias cd='HOME=/vagrant cd'" >> /home/vagrant/.bashrc

echo "Done! :)"
