#!/bin/bash
#chmod +x run.sh

src=$1

#gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli
 


cd /home/elias/Documents/DistributedSystems/System/Client/$1


START=$(date +%s);

 ./cli << 'EOF'
loggin "spanos"
read t5.zip
write t5.zip

exit
EOF

END=$(date +%s);

cd /home/elias/Documents/DistributedSystems/Scenarios;  


wait

echo 'Script run'
exit
