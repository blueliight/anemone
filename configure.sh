#!/bin/bash

# download lua
echo "Downloading lua..."
wget https://www.lua.org/ftp/lua-5.4.7.tar.gz -o lib/lua.tar.gz
mkdir -p lib/lua
tar -xf lib/lua.tar.gz -C lib/lua --strip-components=1

echo "Building lua..."



#wget https://github.com/terralang/terra/releases/download/release-1.2.0/terra-Linux-x86_64-cc543db.tar.xz -O lib/terra.tar.xz
#mkdir -p lib/terra
#tar -xf lib/terra.tar.xz -C lib/terra --strip-components=1