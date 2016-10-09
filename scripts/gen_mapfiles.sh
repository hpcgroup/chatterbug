#!/bin/bash                                                                      
                                                                                 
if [[ $1 == "-h" ]];                                                             
then                                                                             
  echo "gen_mapfiles.sh <numSample> <max_size 0 - 4> <dst_folder>"               
  exit 0                                                                         
fi                                                                               

numSample=$1                                                                     
max_size=$2                                                                      
dest_folder=$3                                                                   
                                                                                 
dfly="7 4 7 1590 1"                                                              
ftree="22 4 22 484 1"                                                            
ht="9 4 9 1200 1"                                                                
slim="15 4 15 722 1"                                                             
                                                                                 
filename="out.multijob.${max_size}"                                              
echo "python ./job_map_gen.py $max_size $numSample 0 multijob.inp.${max_size} >$filename"
python ./job_map_gen.py $max_size $numSample 0 multijob.inp.${max_size} >$filename
                                                                                 
cur_count=0                                                                      
while read -r line;                                                              
do                                                                               
  echo $cur_count                                                                
  numjobs = `wc -l multijob.inp.${max_size}.${cur_count}`                        
  echo "./many_job global.bin 1 $numjobs $line $dfly multijob.inp.${max_size}.${cur_count}"
  echo "./many_job global.bin 1 $numjobs $line $ftree multijob.inp.${max_size}.${cur_count}"
  echo "./many_job global.bin 1 $numjobs $line $ht multijob.inp.${max_size}.${cur_count}"
  echo "./many_job global.bin 1 $numjobs $line $slim multijob.inp.${max_size}.${cur_count}"
  (( cur_count++ ))                                                              
done < "$filename"                                                               
                                                                                 
mv $filename $dest_folder/                                                       
mv multijob.inp.${max_size}.* $dest_folder/                                      
                                                                                 
