if [[ $1 == "-h" ]];                                                             
then                                                                             
  echo "gen_configs.sh <numSample> <min_size> <max_size 0 - 4> <dst_folder>"
  exit 0                            
fi                                  

numSample=$1
min_size=$2
max_size=$3
dstFolder=$4

ic=1

(( numSample-- ))

for blocking in node router;
do
  for mapping in linear rand;
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
      for net in dfly ftree ht slim;
      do
        cur_folder="${dstFolder}/${blocking}/${mapping}/set_${count}/${net}"
        job_count=`ls ${cur_folder}/job* | wc -l | awk '{print $1}'`
        config_all="$cur_folder/config.all"
        echo "${cur_folder}/global.bin" >${config_all}
        echo "${job_count}" >>${config_all}
        (( job_count-- ))
        for job in `seq 0 $job_count`;
        do
          config_name=$cur_folder/config.${job}
          echo "${cur_folder}/global${job}" >${config_name}
          echo "1" >>${config_name} 
          echo "${job_traces[${job}]} $cur_folder/job${job} ${job_sizes[${job}]} $ic" >>${config_name} 
          if [[ "${job_traces[${job}]}" == "a2a"* ]]; then
            echo "M 0 0 5" >>${config_name} 
          else
            echo "M 0 0 1000" >>${config_name} 
          fi
          echo "E 0 user_code 0.0" >>${config_name} 
          echo "${job_traces[$job]} $cur_folder/job${job} ${job_sizes[$job]} $ic" >>${config_all} 
        done
        for job in `seq 0 $job_count`;
        do
          if [[ "${job_traces[${job}]}" == "a2a"* ]]; then
            echo "M 0 0 5" >>${config_name} >>${config_all}
          else
            echo "M ${job} 0 1000" >>${config_all} 
          fi
          echo "E ${job} user_code 0.0" >>${config_all} 
        done
      done
    done
  done
done
