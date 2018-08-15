##############################################################################
# Copyright (c) 2018, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# Written by:
#     Nikhil Jain <nikhil.jain@acm.org>
#     Abhinav Bhatele <bhatele@llnl.gov>
#
# LLNL-CODE-756471. All rights reserved.
#
# This file is part of Chatterbug. For details, see:
# https://github.com/LLNL/chatterbug
# Please also read the LICENSE file for the MIT License notice.
##############################################################################

#!/bin/bash
 
if [[ $1 == "-h" ]];                                                             
then                                                                             
  echo "gen_mapfiles.sh <numSample> <num_perm> <min_size 0 - 4> <max_size 0 - 4> <dst_folder> <seed>"
  exit 0                                                                         
fi                                                                               
                                                                                 
numSample=$1                                                                     
num_perm=$2                                                                      
min_size=$3                                                                      
max_size=$4                                                                      
dest_folder=$5
seed=$6
                                                                   
dfly="7 4 7 1590 1"                                                              
ftree="22 4 22 484 1"                                                            
ht="9 4 9 1200 1"                                                                
slim="15 4 15 722 1"                                                             
ht_2="4 4 4 2744 1"                                                                
                                                                                 
filename="out.multijob.${min_size}.${max_size}"                                              
echo "python ./map_gen.py $min_size $max_size $numSample $num_perm 1 multijob.inp.${min_size}.${max_size} $seed >$filename"
python ./map_gen.py $min_size $max_size $numSample $num_perm 1 multijob.inp.${min_size}.${max_size} $seed >$filename
                                                                                 
cur_count=0         
line_count=0
cat $filename | while read -r line
do 
  inp_file="multijob.inp.${min_size}.${max_size}.${cur_count}"
  numjobs=`wc -l ${inp_file} | awk '{print $1}'`

  if [[ "$((line_count%5))" == "0" ]];
  then 
  echo "./multi_job global.bin 1 $numjobs $line $dfly ${inp_file}"
  ./multi_job global.bin 1 $numjobs $line $dfly ${inp_file}
  mkdir -p $dest_folder/router/linear/set_${cur_count}/dfly
  mv global* $dest_folder/router/linear/set_${cur_count}/dfly
  mv job* $dest_folder/router/linear/set_${cur_count}/dfly

  fi

  if [[ "$((line_count%5))" == "1" ]];
  then 
  echo "./multi_job global.bin 1 $numjobs $line $ftree ${inp_file}"
  ./multi_job global.bin 1 $numjobs $line $ftree ${inp_file}
  mkdir -p $dest_folder/router/linear/set_${cur_count}/ftree
  mv global* $dest_folder/router/linear/set_${cur_count}/ftree
  mv job* $dest_folder/router/linear/set_${cur_count}/ftree

  fi

  if [[ "$((line_count%5))" == "2" ]];
  then 
  echo "./multi_job global.bin 1 $numjobs $line $ht ${inp_file}"
  ./multi_job global.bin 1 $numjobs $line $ht ${inp_file}
  mkdir -p $dest_folder/router/linear/set_${cur_count}/ht
  mv global* $dest_folder/router/linear/set_${cur_count}/ht
  mv job* $dest_folder/router/linear/set_${cur_count}/ht

  fi

  if [[ "$((line_count%5))" == "3" ]];
  then 
  echo "./multi_job global.bin 1 $numjobs $line $slim ${inp_file}"
  ./multi_job global.bin 1 $numjobs $line $slim ${inp_file}
  mkdir -p $dest_folder/router/linear/set_${cur_count}/slim
  mv global* $dest_folder/router/linear/set_${cur_count}/slim
  mv job* $dest_folder/router/linear/set_${cur_count}/slim
  
  fi
 
  if [[ "$((line_count%5))" == "4" ]];
  then 
  echo "./multi_job global.bin 1 $numjobs $line $ht_2 ${inp_file}"
  ./multi_job global.bin 1 $numjobs $line $ht_2 ${inp_file}
  mkdir -p $dest_folder/router/linear/set_${cur_count}/ht-2
  mv global* $dest_folder/router/linear/set_${cur_count}/ht-2
  mv job* $dest_folder/router/linear/set_${cur_count}/ht-2
  
  fi

  (( line_count++ ))
  if [[ "$((line_count%5))" == "0" ]] 
  then
    (( cur_count++ ))
  fi
done 

mv $filename $dest_folder/router
mv multijob.inp.${min_size}.${max_size}.* $dest_folder/router

