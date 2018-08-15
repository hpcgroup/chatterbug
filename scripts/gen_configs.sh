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

if [[ $1 == "-h" ]];                                                             
then                                                                             
  echo "gen_configs.sh <numSample> <min_size> <max_size 0 - 4> <dst_folder>"
  exit 0                            
fi                                  

numSample=$1
min_size=$2
max_size=$3
dstFolder=$4

(( numSample-- ))

for blocking in node router;
do
  if [ ! -d "${dstFolder}/${blocking}" ]; then
    continue
  fi
  for mapping in static pods linear random clustered fixedplane;
  do
    for count in `seq 0 $numSample`;
    do
      config_file="${dstFolder}/${blocking}/multijob.inp.${min_size}.${max_size}.config.${count}"
      jobc=0
      declare -A job_traces
      declare -A job_sizes
      while read trace job_size;
      do
        job_traces[${jobc}]="${trace}"
        job_sizes[${jobc}]="${job_size}"
        (( jobc++ ))
      done < ${config_file}
      for net in dfly ftree ftree-2 ht slim ht-2;
      do
        cur_folder="${dstFolder}/${blocking}/${mapping}/set_${count}/${net}"
        if [ ! -d "${cur_folder}" ]; then
          continue
        fi
        job_count=`ls ${cur_folder}/job* | wc -l | awk '{print $1}'`
        config_all="$cur_folder/config.all"
        echo "${cur_folder}/global.bin" >${config_all}
        echo "${job_count}" >>${config_all}
        (( job_count-- ))
        for job in `seq 0 $job_count`;
        do
          #config_name=$cur_folder/config.${job}
          #echo "${cur_folder}/global${job}" >${config_name}
          #echo "1" >>${config_name} 
          #echo "${job_traces[${job}]} $cur_folder/job${job} ${job_sizes[${job}]} $ic" >>${config_name} 
          #if [[ "${job_traces[${job}]}" == "a2a"* ]]; then
          #  echo "M 0 0 5" >>${config_name} 
          #else
          #  echo "M 0 0 1000" >>${config_name} 
          #fi
          #echo "E 0 user_code 0.0" >>${config_name} 
          if [[ "${job_traces[${job}]}" == "subcom-a2a"* ]]; then
            ic=5
          elif [[ "${job_traces[${job}]}" == "stencil3d"* ]]; then
            ic=4
          elif [[ "${job_traces[${job}]}" == "near-neighbor"* ]]; then
            ic=6
          elif [[ "${job_traces[${job}]}" == "permutation"* ]]; then
            ic=8
          elif [[ "${job_traces[${job}]}" == "spread"* ]]; then
            ic=6
          fi
          echo "${job_traces[$job]} $cur_folder/job${job} ${job_sizes[$job]} $ic" >>${config_all} 
        done
        for job in `seq 0 $job_count`;
        do
          if [[ "${job_traces[${job}]}" == "subcom-a2a"* ]]; then
            echo "M ${job} 100000 1048576" >>${config_all} 
          elif [[ "${job_traces[${job}]}" == "stencil3d"* ]]; then
            echo "M ${job} 100000 10485760" >>${config_all} 
          elif [[ "${job_traces[${job}]}" == "near-neighbor"* ]]; then
            echo "M ${job} 100000 1048576" >>${config_all} 
          elif [[ "${job_traces[${job}]}" == "permutation"* ]]; then
            echo "M ${job} 100000 10485760" >>${config_all} 
          elif [[ "${job_traces[${job}]}" == "spread"* ]]; then
            echo "M ${job} 100000 1048576" >>${config_all} 
          fi
          echo "E ${job} user_code 0.0" >>${config_all} 
        done
      done
    done
  done
done
