#!/bin/bash
#    Begin PBS directives
#PBS -A CSC189
#PBS -N traceR
#PBS -j oe
#PBS -l walltime=3:00:00,nodes=128
#PBS -l gres=atlas1%atlas2

cd /ccs/home/njain/pams_work/profiling/permutation
module unload  craype-hugepages8M
export HUGETLB_MORECORE=no   
export ATP_ENABLED=1   
aprun -n 1024 -N 8 -j 1 ./traceR --sync=3  --extramem=1000000 --nkp=${NKP} --gvt-interval=16 --batch=8 -- ${MACHINE} ${CONF} | tee ${OUT}
