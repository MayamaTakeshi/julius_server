#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail


echo "Creating folder for source"
mkdir -p /usr/local/src
mkdir -p /usr/local/src/git


echo "Installing git lfs"
mkdir -p /root/tmp/git-lfs
cd /root/tmp/git-lfs
rm -f git-lfs-linux-amd64-v2.13.2.tar.gz 
wget https://github.com/git-lfs/git-lfs/releases/download/v2.13.2/git-lfs-linux-amd64-v2.13.2.tar.gz
tar xf git-lfs-linux-amd64-v2.13.2.tar.gz 
./install.sh 


echo "Building julius"
cd /usr/local/src/git
apt -y install build-essential zlib1g-dev libsdl2-dev libasound2-dev
git clone https://github.com/julius-speech/julius.git
cd julius
git checkout 4ba475bd430faa5919e13e2020d6edda1491fb66
./configure --enable-words-int
make -j4


echo "Installing dependencies"
apt -y install libpulse-dev


echo "Installing sox"
apt -y install sox


echo "Get source code"
cd /usr/local/src/git/
git clone https://github.com/julius-speech/dictation-kit
cd dictation-kit/


echo "Success"
