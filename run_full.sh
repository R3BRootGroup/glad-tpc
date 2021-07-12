#!/bin/bash
echo -e "Are you sure? If it's not your first use, this script will overwrite your previous results.\n"
read -p "Type y or Y to proceed:		" -n 1 -r
echo 
if [[ $REPLY =~ ^[Yy]$ ]]; then
cd ./gtpcgen
echo -e "\n----------------Generating the primary events ASCII file!----------------\n"
root -b run_generator.C <<EOF
.q
EOF
echo -e "\n----------------Trasporting the generated events inside the experimental area!----------------\n"
cd ../macros/sim
root -b run_simHYDRA.C <<EOF
.q
EOF
echo -e "\n----------------Projecting into the pad plane the tracks inside the active region!----------------\n"
cd ../proj
root -b run_proj.C<<EOF
.q
EOF
echo -e "\n----------------Processing the projected tracks with the AGET electronics!----------------\n"
cd ../electronics
. run_ele.sh
cd ../../..
echo -e "\n----------------End of the simulation!----------------\n"
fi

