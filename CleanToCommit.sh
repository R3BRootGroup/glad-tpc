#!/usr/bin/env bash
#it's an attempt!!
read -p "are you sure you want to delete the files? Y/N " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
   #rm -i ./gtpcgen/*_cc*
   #rm -i ./gtpcgen/ASCII/*.txt
   #cd ./macros/Analysis
   #find -name Analysis*.ps -delete
   #cd ../sim
   #find -name *.root -delete
   #cd ../proj
   #find -name *.root -delete
fi

