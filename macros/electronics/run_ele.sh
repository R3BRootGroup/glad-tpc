#!/bin/bash
echo -e "\nFYI: to stop this script Ctrl+C doesn't work, kill it using htop for example!\n\n"
echo -e "Are you sure? If it's not your first use, this script will overwrite your previous results.\n"
read -p "Type y or Y to proceed:		" -n 1 -r
echo 
if [[ $REPLY =~ ^[Yy]$ ]]; then
echo -e "\n---------------------Building Electronics_MT.C!---------------------\n"
cd build
make clean
cmake ..
make -j2
echo -e "\n---------------------Running the macro in different cores!---------------------\n"
core=7

for i in `seq 1 $core`;
do
./AGetElectronics $i $core &
done
wait
echo -e "\n---------------------Merging the root output file!---------------------\n"
hadd  -f ../AGET/ele_output.root ../AGET/mt/*.root       
wait
cd ../AGET/mt/
rm *
cd ..
echo -e "\n---------------------The end!---------------------\n"
fi
