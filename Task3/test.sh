#!/bin/bash
#ssh liza@127.0.0.1 'bash -s' < test.sh - эту строчку ввела в терминале для того, чтобы тест сработал на локальном сервере.
export DISPLAY=':0.0'
gedit -s "NewFile" &
sleep 5
STR="HelloWorld!"
GEDITPID=$(pgrep gedit -n)
GEDITWID=$(xdotool search --pid $GEDITPID | tail -1)

xdotool windowactivate $GEDITWID 
xdotool type ${STR}
xdotool key --delay 100 ctrl+s
xdotool key alt+F4

if [ "$STR" == "$(head -c 11 NewFile)" ]; 
then 
echo "Match" 
else 
echo "Not match" 
fi 