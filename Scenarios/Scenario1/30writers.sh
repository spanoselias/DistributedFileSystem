#!/bin/bash
#chmod +x run.sh

src=$1

echo "This script is about to run another script."
 
echo "This script has just run another script."

cd /home/elias/Documents/DistributedSystems/Scenarios 
echo "Starting scenario1 , 30 writers">> senario1.txt 
echo "-----------------------------------------------">> senario1.txt 

cd /home/elias/Documents/DistributedSystems/Scenarios/Scenario1
STARTTIME=$(date +%s)
for ((i=1;i<=1;i++)); do

	sh ./1writer.sh 1 &
	sh ./1writer.sh 1 &
	sh ./1writer.sh 2 &
	sh ./1writer.sh 3 &
	sh ./1writer.sh 4 &
	sh ./1writer.sh 5 &
	sh ./1writer.sh 6 &
	sh ./1writer.sh 7 &
	sh ./1writer.sh 8 &
	sh ./1writer.sh 9 &
	sh ./1writer.sh 10 &
	sh ./1writer.sh 11 &
	sh ./1writer.sh 12 &
	sh ./1writer.sh 13 &
	sh ./1writer.sh 14 &
	sh ./1writer.sh 15 &
	sh ./1writer.sh 16 &
	sh ./1writer.sh 17 &
	sh ./1writer.sh 18 &
	sh ./1writer.sh 19 &
	sh ./1writer.sh 20 & 
	sh ./1writer.sh 21 &
	sh ./1writer.sh 22 &
	sh ./1writer.sh 23 &
	sh ./1writer.sh 24 &
	sh ./1writer.sh 25 &
	sh ./1writer.sh 26 &
	sh ./1writer.sh 27 &
	sh ./1writer.sh 28 &
	sh ./1writer.sh 29 &
	sh ./1writer.sh 30  
	 
done 
 
wait

ENDTIME=$(date +%s)
cd /home/elias/Documents/DistributedSystems/Scenarios

echo "$(($ENDTIME - $STARTTIME))" >> senario1.txt 

echo "-----------------------------------------------">> senario1.txt
echo "Ending scenario1 , 30 writers">> senario1.txt 
echo "-----------------------------------------------">> senario1.txt
 

echo 'Script run'
exit
