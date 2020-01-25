#!/bin/sh -e

# Based on https://medium.com/@george.shuklin/how-to-install-packages-from-a-newer-distribution-without-installing-unwanted-6584fa93208f

echo "deb http://security.ubuntu.com/ubuntu eoan-security main universe" > /etc/apt/sources.list.d/qemu.list

cat > /etc/apt/preferences.d/qemu.pref <<EOF
Package: *
Pin: release n=eoan
Pin-Priority: -10
EOF

apt-get update
