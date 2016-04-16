#!/bin/bash
#chmod +x run.sh

src=$1

nohup node app &
$ echo $! > node-instance.pid

gcc sam.c -o sam
./sam
 
 
exit
