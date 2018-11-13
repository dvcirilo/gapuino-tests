# -*- mode: ruby -*-
# vi: set ft=ruby :

# Box / OS
VAGRANT_BOX = "ubuntu/xenial64"

# Memorable name for your
VM_NAME = 'gapuino'

Vagrant.configure("2") do |config|
  config.vm.box = VAGRANT_BOX
  config.vm.hostname = VM_NAME

  config.vm.provider "virtualbox" do |vb|
    vb.name = VM_NAME
    # Passing the USB port of Gapuino to VM 
    # Check on host with "VBoxManage list usbhost"
    # VendorId: 0x0403 and ProductId: 0x6010
    vb.customize ["modifyvm", :id, "--usb", "on"]
    vb.customize ['usbfilter', 'add', '0', '--target', :id, '--name', 'ESP',
                  '--vendorid', '0x0403', '--productid', '0x6010']
    vb.customize [ "modifyvm", :id, "--uartmode1", "file",
                   File.join(Dir.pwd, ".vagrant/ubuntu-xenial-16.04-cloudimg-console.log") ]
  end
  # Script that installs the software needed
  config.vm.provision :shell, path: "bootstrap.sh"
end
