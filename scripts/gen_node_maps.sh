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
                                                                                 
filename="out.multijob.${min_size}.${max_size}"                                              
echo "python ./map_gen.py $min_size $max_size $numSample $num_perm 0 multijob.inp.${min_size}.${max_size} $seed >$filename"
python ./map_gen.py $min_size $max_size $numSample $num_perm 0 multijob.inp.${min_size}.${max_size} $seed >$filename
                                                                                 
cur_count=0                                                                      
cat $filename | while read -r line
do 
  inp_file="multijob.inp.${min_size}.${max_size}.${cur_count}"
  numjobs=`wc -l ${inp_file} | awk '{print $1}'`

  echo "./many_job global.bin 1 $numjobs $line ${inp_file}"
  ./many_job global.bin 1 $numjobs $line $dfly ${inp_file}
  mkdir -p $dest_folder/node/linear/set_${cur_count}/dfly
  mv global* $dest_folder/node/linear/set_${cur_count}/dfly
  mv job* $dest_folder/node/linear/set_${cur_count}/dfly

  echo "./many_job global.bin 1 $numjobs $line $ftree ${inp_file}"
  ./many_job global.bin 1 $numjobs $line $ftree ${inp_file}
  mkdir -p $dest_folder/node/linear/set_${cur_count}/ftree
  mv global* $dest_folder/node/linear/set_${cur_count}/ftree
  mv job* $dest_folder/node/linear/set_${cur_count}/ftree

  echo "./many_job global.bin 1 $numjobs $line $ht ${inp_file}"
  ./many_job global.bin 1 $numjobs $line $ht ${inp_file}
  mkdir -p $dest_folder/node/linear/set_${cur_count}/ht
  mv global* $dest_folder/node/linear/set_${cur_count}/ht
  mv job* $dest_folder/node/linear/set_${cur_count}/ht

  echo "./many_job global.bin 1 $numjobs $line $slim ${inp_file}"
  ./many_job global.bin 1 $numjobs $line $slim ${inp_file}
  mkdir -p $dest_folder/node/linear/set_${cur_count}/slim
  mv global* $dest_folder/node/linear/set_${cur_count}/slim
  mv job* $dest_folder/node/linear/set_${cur_count}/slim

  #rand placement
  echo "./many_job global.bin 2 $numjobs $line $dfly ${inp_file}"
  ./many_job global.bin 2 $numjobs $line $dfly ${inp_file}
  mkdir -p $dest_folder/node/rand/set_${cur_count}/dfly
  mv global* $dest_folder/node/rand/set_${cur_count}/dfly
  mv job* $dest_folder/node/rand/set_${cur_count}/dfly

  echo "./many_job global.bin 2 $numjobs $line $ftree ${inp_file}"
  ./many_job global.bin 2 $numjobs $line $ftree ${inp_file}
  mkdir -p $dest_folder/node/rand/set_${cur_count}/ftree
  mv global* $dest_folder/node/rand/set_${cur_count}/ftree
  mv job* $dest_folder/node/rand/set_${cur_count}/ftree

  echo "./many_job global.bin 2 $numjobs $line $ht ${inp_file}"
  ./many_job global.bin 2 $numjobs $line $ht ${inp_file}
  mkdir -p $dest_folder/node/rand/set_${cur_count}/ht
  mv global* $dest_folder/node/rand/set_${cur_count}/ht
  mv job* $dest_folder/node/rand/set_${cur_count}/ht

  echo "./many_job global.bin 2 $numjobs $line $slim ${inp_file}"
  ./many_job global.bin 2 $numjobs $line $slim ${inp_file}
  mkdir -p $dest_folder/node/rand/set_${cur_count}/slim
  mv global* $dest_folder/node/rand/set_${cur_count}/slim
  mv job* $dest_folder/node/rand/set_${cur_count}/slim

  (( cur_count++ ))
done 

mv $filename $dest_folder/node
mv multijob.inp.${min_size}.${max_size}.* $dest_folder/node

