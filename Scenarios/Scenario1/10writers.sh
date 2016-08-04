#!/bin/bash
#chmod +x run.sh

 

echo "This script is about to run another script."


cd /home/elias/Documents/DistributedSystems/Scenarios 
echo "Starting scenario1 , 10 writers">> senario1.txt 
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
	sh ./1writer.sh 10  	 
done
wait


cd /home/elias/Documents/DistributedSystems/Scenarios 
ENDTIME=$(date +%s)
echo "$(($ENDTIME - $STARTTIME))" >> senario1.txt 

echo "-----------------------------------------------">> senario1.txt
echo "Ending scenario1 , 10 writers">> senario1.txt 
echo "-----------------------------------------------">> senario1.txt
 
wait
echo 'Script run'
exit
