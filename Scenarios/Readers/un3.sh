#!/bin/bash
#chmod +x run.sh

src=$1

echo "This script is about to run another script."
 
echo "This script has just run another script."
cd /home/elias/Documents/DistributedSystems/Client; sh ./unitest.sh   

 
wait
echo 'Script run'
exit
