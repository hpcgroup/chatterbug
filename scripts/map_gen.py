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

from __future__ import print_function
from sys import argv
import random

if(argv[1] == "-h"):
  print ("Usage: job_map_gen.py <min_size 0 - 4> <max_size 0 - 4> <num samples> <num permutations > <output type 0 - node count, 1 - router count> <prefix to output file> <seed>")
  quit()

job_sizes = [ 2000, 4000, 8000, 12000, 16000 ]
#job_names = [ "near-neighbor", "permutation", "spread", "subcom-a2a", "stencil3d" ]
job_names = [ "near-neighbor", "spread", "subcom-a2a", "stencil3d" ]
max_job_types = len(job_names)
trace_name = [ "traces_n2000", "traces_n4000", "traces_n8000", "traces_n12000",
               "traces_n16000" ]
job_shapes = [ [ 10, 10, 20 ], [ 10, 20, 20 ], [ 20, 20, 20],
               [ 20, 20, 30 ], [ 20, 20, 40 ] ]
nodes_per_router = [ 7, 22, 9, 15, 4 ]
max_routers = [ 1590, 484, 1200, 722, 2744 ]

min_size = int(argv[1])
max_size = int(argv[2])
num_samples = int(argv[3])
num_perm = int(argv[4])
out_type = int(argv[5])
out_file = argv[6]

random.seed(int(argv[7]))
max_ranks = 40000
cur_count = 0
while(num_samples > 0):
  indices = [ ]
  types = [ ]
  routers = [ 0, 0, 0, 0, 0 ]
  ranks = 0
  while(ranks < max_ranks):
    next_index = random.randint(min_size, max_size)
    if(ranks + job_sizes[next_index] <= max_ranks):
      next_job_type = random.randint(0, max_job_types-1)
      if(types.count(next_job_type) == 2):
        continue
      indices.append(next_index)
      types.append(next_job_type)
      ranks += job_sizes[next_index]
      for i in range(5):
        routers[i] += job_sizes[next_index]/4/nodes_per_router[i] + 1
      if(max_ranks - ranks < job_sizes[min_size]):
        break

  idx = range(0, len(indices))
  for perm in range(num_perm):
    f = open(out_file+"."+str(cur_count), "w")
    fc = open(out_file+".config."+str(cur_count), "w")
    if(out_type == 0 or out_type == 1):
      if(out_type == 0):
        s = ""
        for i in range(len(indices)):
          s += str(job_sizes[indices[idx[i]]]/4) + " "
        print (s)
      if(out_type == 1):
        for net in range(5):
          if(routers[net] > max_routers[net]):
            print ("Needed more routers than available")
            quit()
          s = ""
          for i in range(len(indices)):
            s += str(job_sizes[indices[idx[i]]]/4/nodes_per_router[net] + 1) + " "
          print (s)
      for i in range(len(indices)):
        s = str(job_sizes[indices[idx[i]]]) + " 2 "
        for j in range(3):
          s += str(job_shapes[indices[idx[i]]][j]) + " "
        s += "1 2 2 1 1 1\n"
        f.write(s)
        fc.write(job_names[types[idx[i]]] + "/" + trace_name[indices[idx[i]]] + " " + str(job_sizes[indices[idx[i]]]))
        fc.write("\n")
    f.close()
    fc.close()
    cur_count += 1
    random.shuffle(idx)
  num_samples -= 1
