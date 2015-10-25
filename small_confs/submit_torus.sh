#!/bin/bash

NKP_DFLY=672
NKP_TORUS=408
size=2000K
network=torus

if [ "${network}" = 'torus' ]; then
  nkp_val=${NKP_TORUS};
elif [ "${network}" = 'dfly' ]; then
  nkp_val=${NKP_DFLY};
elif [ "${network}" = 'fattree' ]; then
  nkp_val=${NKP_FATTREE};
fi

for packet in 1024
do
  for sched in fcfs
  do
    for delay in 25 
    do
      for lband in 2 5 10 20 50 1000 
      do
        for cnband in 5
        do
          for buffer in 65536
          do
            for routing in adaptive
            do
              conf=${network}_${size}_${packet}_${sched}_${delay}_${lband}_${cnband}_${buffer}_${routing}
              linkOut="linkData/${conf}"
              echo "mkdir ${linkOut}"
              mkdir ${linkOut}
              cat > ${conf} <<END_CONF
LPGROUPS
{
   MODELNET_GRP
   {
      repetitions="24576";
      server="16";
      modelnet_torus="1";
   }
}
PARAMS
{
   packet_size="$packet";
   chunk_size="$packet";
   modelnet_order=( "torus" );
   # scheduler options
   modelnet_scheduler="$sched";
   message_size="300";
   router_delay="$delay";
   soft_delay="1000";
   n_dims="5";
   dim_length="8,8,12,16,2";
   link_bandwidth="$lband";
   cn_bandwidth="$cnband";
   buffer_size="$buffer";
   num_vc="2";
   routing="$routing";
}

END_CONF
              echo "NKP=${nkp_val} TRACER_LINK_FILE=${linkOut}/link MACHINE=${conf} CONF=config_${network}_${size} OUT=sumData/out-${conf} qsub -V script_arg.sh"
              NKP=${nkp_val} TRACER_LINK_FILE=${linkOut}/link MACHINE=${conf} CONF=config_${network}_${size} OUT=sumData/out-${conf} qsub -V script_arg.sh
            done
          done
        done
      done
    done
  done
done
