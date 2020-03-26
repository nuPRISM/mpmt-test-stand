#!/bin/bash

# Exit when any command fails
set -e

# Check for root
if [[ $EUID -eq 0 ]]; then
   echo "This script is NOT meant to be run as root" 
   exit 1
fi

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <start step>"
    echo ""
    echo "    <start step> = integer indicating which step of the install process to start with"
    echo "                   (all prior steps will be skipped)"
    echo "        0 = Install Prerequisites"
    echo "        1 = Update udev rules"
    echo "        2 = Install PlatformIO"
    echo "        3 = Update PATH"
    exit 0
fi

START=$1

# Install pre-requisites
if [ "$START" -le "0" ]; then
    echo "0: Install prerequisites..."
    sudo apt-get update
    sudo apt-get install curl -y
    sudo apt-get install python3-distutils -y
fi

# Update udev rules
if [ "$START" -le "1" ]; then
    echo "1: Update udev rules..."
    curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/master/scripts/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
    sudo service udev restart
    sudo usermod -aG dialout $USER
fi

# Install PlatformIO
if [ "$START" -le "2" ]; then
    echo "2: Install PlatformIO..."
    PIO_INSTALLER=get-platformio.py

    # Download PlatformIO installer script
    curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o $PIO_INSTALLER
    # Run the installer script
    python3 $PIO_INSTALLER
    # Delete the installer script
    rm -f $PIO_INSTALLER
fi

# Add PlatformIO tools to path
if [ "$START" -le "3" ]; then
    echo "3: Update PATH..."
    echo "export PATH=\$PATH:~/.platformio/penv/bin" >> ~/.bashrc
fi

echo "Done"
