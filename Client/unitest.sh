#!/bin/bash
#chmod +x run.sh

src=$1

gcc client.c  `pkg-config --cflags --libs glib-2.0` -lm -lpthread -o cli

 
 ./cli << 'EOF'
loggin spanos2
list
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
write img.png
read img.png
exit
EOF

 

 
 


wait

echo 'Script run'
exit
