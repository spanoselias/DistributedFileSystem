#!/bin/bash
#chmod +x run.sh

src=$1

gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
./cli <<< 'read s1.mp3' &
./cli <<< 'read s2.mp3' &
./cli <<< 'read t5.zip' &
./cli <<< 'read t10.zip'


wait

echo 'Script run'
exit
