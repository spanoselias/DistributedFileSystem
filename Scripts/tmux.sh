#!/bin/bash
#chmod +x run.sh

tmux new -s servers
tmux split-window -h prefix + %
