#!/bin/bash

set -e

echo "Downloading DASM"

curl -o /tmp/dasm.tar.gz --location https://github.com/dasm-assembler/dasm/releases/download/2.20.14.1/dasm-2.20.14.1-osx-x64.tar.gz
rm -r dasm/ || true
mkdir dasm/
tar -xzf /tmp/dasm.tar.gz -C dasm/

echo "Installed DASM; check that you see a version below!"

dasm/dasm | head -n 2

echo "Installing Pyserial"

python3 -m pip install pyserial

