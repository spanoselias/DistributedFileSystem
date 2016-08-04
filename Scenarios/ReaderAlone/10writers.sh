#!/bin/bash
#chmod +x run.sh

src=$1

echo "This script is about to run another script."
 
echo "This script has just run another script."

 

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
	sh ./1writer.sh 14 
   
 
 
wait
echo 'Script run'
exit
