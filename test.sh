#!/bin/sh

export LD_LIBRARY_PATH=./
echo "----- 0 -----"
killall stored
./stored
sleep 1

export LD_PRELOAD=./ldhook.so
echo "----- 1 -----"
echo 'This is a test file.' > hello.txt
./open_close hello.txt

echo "----- 2 -----"
echo 'This is a test file.' > hello.txt
/bin/cp -f hello.txt aaa.txt

