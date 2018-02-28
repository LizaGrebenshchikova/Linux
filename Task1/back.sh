#!/bin/bash

SCRIPTNAME=$0
TMPDIRNAME=$1
ARCHNAME=$2

#Save current directory.
CURRDIRECTORY=$(pwd)

mkdir $TMPDIRNAME
cd $TMPDIRNAME
DIRTOCOPY=$(pwd)
cd

#Regular expressions for the extensions.
FILESNAME=".*\.("
FILESNAME=$FILESNAME"$3"
#Iterations in the extensions, if possible.
for i in "${@:4}"
do  
FILESNAME=$FILESNAME"|$i"
done
FILESNAME=$FILESNAME")$"

for j in $(find * -regextype posix-egrep -regex $FILESNAME -type f)
do
RENAME=$(echo $j | sed 's/\///')
cp $j $DIRTOCOPY"/"$RENAME
done

cd $CURRDIRECTORY
tar -cjf $ARCHNAME $TMPDIRNAME

echo "Done"
