#!/bin/bash

# Exit when any command fails
set -e

export MIDASSYS=$HOME/packages/midas
export MIDAS_EXPTAB=$HOME/online/exptab
export MIDAS_EXPT_NAME=mpmttest
export MIDAS_PATH=$MIDASSYS/bin

export PATH=$PATH:$MIDAS_PATH

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <start step>"
    echo ""
    echo "    <start step> = integer indicating which step of the install process to start with"
    echo "                   (all prior steps will be skipped)"
    echo "        0 = Install Prerequisites"
    echo "        1 = Set environment variables in .bashrc"
    echo "        2 = Clone MIDAS repo"
    echo "        3 = Build MIDAS"
    echo "        4 = Setup experiment"
    echo "        5 = Initialize ODB"
    echo "        6 = Create SSL certificate"
    echo "        7 = Create password file"
    exit 0
fi

START=$1

echo "Killing running MIDAS processes..."
if pgrep mhttpd; then killall mhttpd; fi

if [ "$START" -le "0" ]; then
    echo "0: Installing prerequesites..."
    sudo apt-get update
    sudo apt-get install build-essential cmake -y
    sudo apt-get install zlib1g zlib1g-dev -y
    sudo apt-get install openssl libssl-dev -y
    sudo apt-get install sqlite3 libsqlite3-dev -y
fi

if [ "$START" -le "1" ]; then
    echo "1: Setting environment variables..."
    echo "export MIDASSYS=$MIDASSYS" >> ~/.bashrc
    echo "export MIDAS_EXPTAB=$MIDAS_EXPTAB" >> ~/.bashrc
    echo "export MIDAS_EXPT_NAME=$MIDAS_EXPT_NAME" >> ~/.bashrc
    echo "export PATH=\$PATH:$MIDAS_PATH" >> ~/.bashrc
fi

# Install MIDAS and other packages
if [ "$START" -le "2" ]; then
    echo "2: Cloning MIDAS..."
    mkdir $HOME/packages
    cd $HOME/packages
    git clone https://bitbucket.org/tmidas/midas --branch midas-2020-03-a --depth 1 --recursive
fi

# Build MIDAS
if [ "$START" -le "3" ]; then
    echo "3: Building MIDAS..."
    cd $MIDASSYS
    make clean
    rm -rf build

    mkdir build
    cd build
    cmake ..
    make install
    cd ..

    ODBEDIT=bin/odbedit
    if [ -f $ODBEDIT ]; then
        echo "File $ODBEDIT exists."
    else
        echo "File $ODBEDIT does not exist."
        exit 1
    fi
fi

# Set up Experiment
if [ "$START" -le "4" ]; then
    echo "4: Setting up Experiment..."
    rm -rf $HOME/online
    mkdir $HOME/online
    cd $HOME/online

    echo "$MIDAS_EXPT_NAME $HOME/online $USER" > $MIDAS_EXPTAB
fi

# Configure MIDAS
if [ "$START" -le "5" ]; then
    echo "5: Initializing ODB..."
    odbedit -c exit
fi

if [ "$START" -le "6" ]; then
    echo "6: Creating SSL certificate..."
    cd $HOME/online
    openssl req -new -nodes -newkey rsa:2048 -sha256 -out ssl_cert.csr -keyout ssl_cert.key -subj '/CN=localhost'
    openssl x509 -req -days 365 -sha256 -in ssl_cert.csr -signkey ssl_cert.key -out ssl_cert.pem
    cat ssl_cert.key >> ssl_cert.pem
fi

if [ "$START" -le "7" ]; then
    echo "7: Creating password file..."
    MIDAS_USER_NAME=midas
    MIDAS_USER_REALM=$MIDAS_EXPT_NAME
    MIDAS_USER_PW=midas
    digest="$( printf "%s:%s:%s" "$MIDAS_USER_NAME" "$MIDAS_USER_REALM" "$MIDAS_USER_PW" |
               md5sum | awk '{print $1}' )"
    printf "%s:%s:%s\n" "$MIDAS_USER_NAME" "$MIDAS_USER_REALM" "$digest" > "$HOME/online/htpasswd.txt"
    echo "MIDAS Username: $MIDAS_USER_NAME"
    echo "MIDAS Password: $MIDAS_USER_PW"
fi

echo "Done"
