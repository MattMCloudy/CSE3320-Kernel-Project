#!/bin/bash

make clean;
make menuconfig;
make -j4 && make modules -j4 && echo 'MAKE WORKED';
make modules install -j4 && make install -j4 && reboot;
