#!/bin/sh

# Place this script in the GameLift server package
# GameLift will run this script before opening the server
# https://docs.o3de.org/docs/user-guide/gems/reference/aws/aws-gamelift/build-packaging-for-linux/

sudo cp -a ./lib64/libX* /lib64/.
sudo cp -a ./lib64/libbsd* /lib64/.
sudo cp -a ./lib64/libstdc++* /lib64/.

sudo yum update -y
sudo yum groupinstall 'Development Tools' -y
sudo yum install python3 -y
sudo yum -y install libunwind-devel

echo 'Update outdated make package'
mkdir make && cd make
wget http://ftp.gnu.org/gnu/make/make-4.2.1.tar.gz
tar zxvf make-4.2.1.tar.gz
mkdir make-4.2.1-build make-4.2.1-install
cd make-4.2.1-build
/local/game/make/make-4.2.1/configure --prefix='/local/game/make/gmake-4.2.1-install'
make -j 8
make -j 8 install
export PATH=/local/game/make/gmake-4.2.1-install/bin:$PATH
sudo ln -sf /local/game/make/gmake-4.2.1-install/bin/make /local/game/make/gmake-4.2.1-install/bin/gmake
cd /local/game/

echo 'Installing missing libs for AL2023'
mkdir glibc && cd glibc
wget http://mirror.rit.edu/gnu/libc/glibc-2.29.tar.gz
tar zxvf glibc-2.29.tar.gz
mkdir glibc-2.29-build glibc-2.29-install
cd glibc-2.29-build
/local/game/glibc/glibc-2.29/configure --prefix='/local/game/glibc/glibc-2.29-install'
make -j 8
make -j 8 install
sudo ln -sf /local/game/glibc/glibc-2.29-install/lib/libm.so.6 /local/game/lib64/libm.so.6
cd /local/game/


echo 'Install Success'
