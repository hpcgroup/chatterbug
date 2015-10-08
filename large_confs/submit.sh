#!/bin/bash

NKP_DFLY=4394
NKP_TORUS=2962
NKP_FATTREE=2963
#for size in 5K
for size in 5K
do
  for routing in dyn_rr
  do
    for network in fattree
    do
      if [ "${network}" = 'torus' ]; then
        nkp_val=${NKP_TORUS};
      elif [ "${network}" = 'dfly' ]; then
        nkp_val=${NKP_DFLY};
      elif [ "${network}" = 'fattree' ]; then
        nkp_val=${NKP_FATTREE};
      fi
      linkOut="linkData/${network}_${routing}_${size}";
      echo "mkdir ${linkOut}"
      mkdir ${linkOut}
      echo "NKP=${nkp_val} TRACER_LINK_FILE=${linkOut}/link MACHINE=${network}_${routing}.conf CONF=config_${network}_${size} OUT=sumData/out-${network}_${routing}_${size} qsub -V script_arg.sh"
      NKP=${nkp_val} TRACER_LINK_FILE=${linkOut}/link MACHINE=${network}_${routing}.conf CONF=config_${network}_${size} OUT=sumData/out-${network}_${routing}_${size} qsub -V script_arg.sh
    done
  done
done
