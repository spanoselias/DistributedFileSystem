#!/bin/bash
#chmod +x run.sh

src=$1

#gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
 


cd /home/elias/Documents/DistributedSystems/System/Client/$1


START=$(date +%s);

 ./cli << 'EOF'
loggin "spanos"
read v1.mp4
write v1.mp4
exit
EOF

END=$(date +%s);

cd /home/elias/Documents/DistributedSystems/Scenarios;  
echo $((END-START)) | awk '{print int($1/60)":"int($1%60)}' >> senario1.txt   


wait

echo 'Script run'
exit
