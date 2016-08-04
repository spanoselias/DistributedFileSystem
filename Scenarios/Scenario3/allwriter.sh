#!/bin/bash
#chmod +x run.sh

 

echo "This script is about to run another script."


cd /home/elias/Documents/DistributedSystems/Scenarios 
echo "Starting Scenario3">> senario3.txt 
echo "-----------------------------------------------">> senario3.txt 
 
cd /home/elias/Documents/DistributedSystems/Scenarios/Scenario3
echo "This script has just run another script."
STARTTIME=$(date +%s)
for ((i=1;i<=1;i++)); do

	sh ./1writer.sh 1 &
	sh ./1writer.sh 2
 	 
done  

wait 

cd /home/elias/Documents/DistributedSystems/Scenarios
echo "----------------">> senario3.txt
echo "Ending for s2.mp3">> senario3.txt 
echo "-------------------">> senario3.txt


echo "Starting for v1.mp4">> senario3.txt 
cd /home/elias/Documents/DistributedSystems/Scenarios/Scenario3
for ((i=1;i<=1;i++)); do

	sh ./2writer.sh 1 &
	sh ./2writer.sh 2
 	 
done  

wait 
ENDTIME=$(date +%s)



cd /home/elias/Documents/DistributedSystems/Scenarios
echo "$(($ENDTIME - $STARTTIME))" >> senario3.txt
echo "----------------">> senario3.txt
echo "Ending for v1.mp4">> senario3.txt 
echo "-------------------">> senario3.txt

echo "Starting for t1.zip">> senario3.txt 
echo "-------------------">> senario3.txt
cd /home/elias/Documents/DistributedSystems/Scenarios/Scenario3
STARTTIME=$(date +%s)
for ((i=1;i<=1;i++)); do

	sh ./3writer.sh 1 &
	sh ./3writer.sh 2 
	
 	 
done  

wait 
ENDTIME=$(date +%s)
cd /home/elias/Documents/DistributedSystems/Scenarios
echo "$(($ENDTIME - $STARTTIME))" >> senario3.txt

echo "----------------">> senario3.txt
echo "Ending for t1.zip">> senario3.txt 
echo "-------------------">> senario3.txt


cd /home/elias/Documents/DistributedSystems/Scenarios

echo "Starting for t5.zip">> senario3.txt 
echo "-------------------">> senario3.tx



cd /home/elias/Documents/DistributedSystems/Scenarios 
echo "----------------">> senario3.txt
echo "Ending for t5.zip">> senario3.txt 
echo "----------------">> senario3.txt
 
wait
echo 'Script run'
exit
