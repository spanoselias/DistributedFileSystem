#!/bin/bash
#chmod +x run.sh

src=$1

gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
./cli s1 mp3 spanos &
./cli s1 mp3 spa  &
./cli s1 mp3 spa1 &
./cli s1 mp3 spa2 &
./cli s1 mp3 spa3 &
./cli s1 mp3 spa4 &
 
 


wait

echo 'Script run'
exit
