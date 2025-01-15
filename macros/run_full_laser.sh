#!/bin/bash
echo -e "Are you sure? If it's not your first use, this script will overwrite your previous results.\n"
read -p "Type y or Y to proceed:		" -n 1 -r
echo 
if [[ $REPLY =~ ^[Yy]$ ]]; then
cd ./sim
echo -e "\n----------------Simulating!----------------\n"
root -b run_simHYDRA_laser.C <<EOF
.q
EOF
echo -e "\n----------------Projecting into the pad plane the tracks inside the active region!----------------\n"
cd ../proj
root -l 'run_laser_gen.C(kTRUE)' <<EOF
.q
EOF
echo -e "\n----------------Representing the results in the pad plane----------------\n"
cd ../vis
root -l run_laserVis.C
echo -e "\n----------------Projecting into the pad plane (again) the tracks inside the active region!----------------\n"
cd ../proj
root -l run_laser_gen.C <<EOF
.q
EOF
echo -e "\n----------------Performing the track reconstruction!----------------\n"
cd ../reco
root -l run_reconstruction_laser.C <<EOF
.q
EOF
echo -e "\n----------------Comparing with the generated laser!----------------\n"
root -l residues_laser.C
echo -e "\n----------------End of the simulation!----------------\n"
fi

