#!/bin/bash
#!/usr/bin/expect
#chmod +x run.sh

pass=$1 

#gnome-terminal  -x bash -c "ssh -t espano01@b103ws6.in.cs.ucy.ac.cy; awk '{\$1}' "

#ssh -l espano01@b103ws6.in.cs.ucy.ac.cy  "$1; bash"

#gnome-terminal  -x bash -c "ssh -t espano01@b103ws6.in.cs.ucy.ac.cy; $1 "
 
#gnome-terminal  -x bash -c "sshpass -p '$1' ssh -t espano01@b103ws6.in.cs.ucy.ac.cy; "

 
#gnome-terminal  -x bash -c "sshpass -p '$1' ssh -t espano01@b103ws6.in.cs.ucy.ac.cy;" 

gnome-terminal  -e "sshpass -p '$1' ssh -t espano01@b103ws6.in.cs.ucy.ac.cy bash -c 'pwd ; pwd ; cd Desktop && bash -l '" 


#gnome-terminal  -x bash -c "sshpass -p '$1' ssh -o StrictHostKeyChecking=no espano01@b103ws6.in.cs.ucy.ac.cy ''; cat"

 

echo 'Script run'
exit


gnome-terminal  -e "sshpass -p '$1' ssh -t espano01@b103ws6.in.cs.ucy.ac.cy bash -c 'pwd ; pwd ; cd /home/students/cs/2013/espano01/Desktop/git/server1/Client && bash -l '" 
