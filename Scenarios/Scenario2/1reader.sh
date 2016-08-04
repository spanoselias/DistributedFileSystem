#!/bin/bash
#chmod +x run.sh

src=$1

#gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
 


cd /home/elias/Documents/DistributedSystems/System/Client/$1


STARTTIME=$(date +%s) 

 ./cli << 'EOF'
loggin "spanos"
read img.png 
exit
EOF
 
ENDTIME=$(date +%s)

cd /home/elias/Documents/DistributedSystems/Scenarios;  
 
#echo "$(($ENDTIME - $STARTTIME))" >> senario2.txt 

wait

echo 'Script run'
exit
