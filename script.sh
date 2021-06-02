#!/bin/bash
for i in $(seq 1 10)
do
	./lahc < Instances/Deroussi_Norre_2010/"fjsp$i" > Experiments_Outputs/LAHC/"out_fjsp$i".csv
	./ils < Instances/Deroussi_Norre_2010/"fjsp$i" > Experiments_Outputs/ILS/"out_fjsp$i".csv
done
