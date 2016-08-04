#***********************************************************************************/
#*                                                                                 */
#*Name: Elias Spanos                                                 			         */
#*Date: 16/04/2016                                                                 */
#*Filename: start-up remote servers                                                */
#*                                                                                 */
#***********************************************************************************/

#!/bin/bash
#!/usr/bin/expect
#chmod +x run.sh

pass=$1 

END=$2 


repliport=30005
dirport=20005
managerport=40002 

 
for ((i=1;i<=END;i++)); do 

getent hosts b103ws$i.in.cs.ucy.ac.cy | awk '{print $1 }' >> tem.txt

done

for ((i=1;i<=END;i++)); do 
   	
  #String name in order to create a directory in the remote
  #server
  STRDir="Directory_"
  STRDir+=$i;  
	
	#build-up the port that the server will listen
  dirport=$(($dirport + $i))  

  #This is the defualt directory that the script command is going to 
  #connect in the remote server and execute the appropriate server
	path="cd /home/students/cs/2013/espano01/DistributedSystem"

	#copypath="scp -r /home/elias/Documents/DistributedSystems/Client  espano01@b103ws1:/home/students/cs/2013/espano01/DistributedSystem/Directory_1"

  #command in order to create the folder in the remote server
	folder="mkdir -p  System/"$STRDir"" 	
  
 gnome-terminal  -e "sshpass -p '$1' ssh -t espano01@b103ws$i.in.cs.ucy.ac.cy bash -c 'pwd ; pwd ; $path;  mkdir -p  "$STRDir"   && bash -l '"
   
done
 



 

echo 'Script run'
exit
