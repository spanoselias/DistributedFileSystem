#!/bin/bash
#chmod +x run.sh

rm  myfile.txt

repliport=30005
dirport=20005
managerport=40002 
 
END=$1

echo "#DIRECTORIES" >> myfile.txt
echo "$1" >> myfile.txt
for ((i=1;i<=END;i++)); do
 
   	
  STRDir="Directory_"
  STRDir+=$i;  
	
  dirport=$(($dirport + $i))  

  
  echo "$2 $dirport " >> myfile.txt	
 	
	mkdir -p  System/"$STRDir" 	
	cp -r "Directory/"/* "System/$STRDir"
	gnome-terminal -x sh -c "cd System/$STRDir ; ./dir -p $dirport; cat"
done


echo "#REPLICAS" >> myfile.txt
echo "$1" >> myfile.txt 
for ((i=1;i<=END;i++)); do
 
  STRRep="Replica_"
  STRRep+=$i; 
  repliport=$(($repliport + $i)) 
 
  echo "$2 $repliport " >> myfile.txt
	 	
  mkdir -p  System/"$STRDir" 	

  mkdir -p  System/"$STRRep" 
	cp -r "Replica/"/* System/$STRRep
	 
	gnome-terminal -x sh -c "cd System/$STRRep ; ./repli -p $repliport; cat"
done


echo "#FILEMANAGERS" >> myfile.txt
echo "$1" >> myfile.txt 
for ((i=1;i<=END;i++)); do
 
  STRManager="FileManager_"
  STRManager+=$i;
  managerport=$(($managerport + $i))  

	echo "$2 $managerport " >> myfile.txt 	
	 	 	
	mkdir -p  System/"$STRManager" 	
  cp -r "FileManager/"/*  System/$STRManager
	 
	gnome-terminal -x sh -c "cd System/$STRManager ; ./fil -p $managerport; cat"
  	
done

echo "#PERMISSION" >> myfile.txt
echo "L0" >> myfile.txt
 

echo 'Script run'
exit
