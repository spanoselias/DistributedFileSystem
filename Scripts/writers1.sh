#!/bin/bash
#chmod +x run.sh

src=$1

gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
./cli <<< 'write s1.mp3' &
./cli <<< 'write s1.mp3' &
./cli <<< 'write s1.mp3' &
./cli <<< 'write s1.mp3' &
./cli <<< 'write s1.mp3'

wait

echo 'Script run'
exit
