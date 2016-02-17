#!/bin/bash
#chmod +x run.sh

gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
./cli <<< 'reader m1.txt' 


wait

echo 'Script run'
exit
