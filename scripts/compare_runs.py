#!/usr/bin/env python                                                          
                                                                               
from sys import argv                                                           
import glob                                                                    
import numpy as np                                                             
np.set_printoptions(threshold=np.nan)

def readfile(filename, cols):                                                       
  ifile = open(filename, 'r')                                               
  data = np.loadtxt(ifile, usecols=cols, skiprows=1)
  flatten = data.flatten()
  return flatten  

allcounters=glob.glob(argv[1]+"/counters.*")
network=argv[2]
if(network == "dfly"):
  start = 3
  end = 25
  fname = "dragonfly-router-traffic"
elif(network == "ftree"):
  start = 3
  end = 47
  fname = "fattree-switch-traffic"
elif(network == "ht"):
  start = 2
  end = 40
  fname = "ht-router-traffic"
else:
  start = 3
  end = 32
  fname = "slimfly-router-traffic"
cols=range(start, end)

#alldata = readfile(argv[1]+"/counters.job.all/"+fname, cols)
#allnz = np.nonzero(alldata)[0]

nzs = [ ] 
for job in range(len(allcounters)-1):
  newdata = readfile(argv[1]+"/counters.job."+str(job)+"/"+fname, cols)
  nzs.append(np.nonzero(newdata)[0])

worst = 0
allusedlink = np.array([ ], dtype=int)
allintersect = np.array([ ], dtype=int) 
for job in range(len(allcounters)-1):
  allusedlink = np.concatenate((nzs[job], allusedlink))
  merge =  np.array([ ], dtype=int)
  for p in range(len(allcounters)-1):
    if(p == job): 
      continue
    intersect = np.intersect1d(nzs[job], nzs[p])
    merge = np.concatenate((merge, intersect))
  merge = np.unique(merge)
  allintersect =  np.concatenate((allintersect, merge))
  if((100*len(merge)/len(nzs[job])) > worst):
    worst = (100*len(merge)/len(nzs[job]))

bincount = np.bincount(allintersect) 
if len(bincount) == 0:
  maxshared = 0
else:
  maxshared = bincount[bincount.argmax()]
print str(worst)+" "+str(100*len(np.unique(allintersect))/len(np.unique(allusedlink)))+" "+str(maxshared)
