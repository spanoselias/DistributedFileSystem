#!/bin/bash
#chmod +x run.sh

src=$1

gnome-terminal -x sh -c "ssh espano01@clusterCG1.in.cs.ucy.ac.cy; cat"

echo 'Script run'
exit
