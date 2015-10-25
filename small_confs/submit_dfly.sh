#!/bin/bash

NKP_DFLY=672
NKP_TORUS=408
size=2000K
network=dfly

if [ "${network}" = 'torus' ]; then
  nkp_val=${NKP_TORUS};
elif [ "${network}" = 'dfly' ]; then
  nkp_val=${NKP_DFLY};
elif [ "${network}" = 'fattree' ]; then
  nkp_val=${NKP_FATTREE};
fi

gband[2]="2.6"
gband[5]="6.5"
gband[10]="13"
gband[20]="26"
gband[50]="65"
gband[1000]="1300"

for packet in 1024
do
  for sched in fcfs
  do
    for delay in 40 
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
      repetitions="4020";
      server="160";
      modelnet_dragonfly="10";
      dragonfly_router="1";
   }
}
PARAMS
{
   packet_size="$packet";
   chunk_size="$packet";
   modelnet_order=( "dragonfly" );
   # scheduler options
   modelnet_scheduler="$sched";
   message_size="512";
   router_delay="$delay";
   soft_delay="1000";
   num_routers="20";
   local_vc_size="$buffer";
   global_vc_size="$buffer";
   cn_vc_size="65536";
   local_bandwidth="$lband";
   global_bandwidth="${gband[$lband]}";
   cn_bandwidth="$cnband";
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
