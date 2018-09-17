Chatterbug v1.0
===============

A suite of communication-intensive proxy applications that mimic commonly found
communication patterns in HPC codes. These codes can be used as synthetic codes
for benchmarking, or for trace generation using OTF2.

### List of Proxy Applications:

  * pairs: all pairs multi-pairs message exchange
  * ping-ping: multi-pairs message exchange (user specified pairs)
  * spread: k-neighbor communication within rankspace neighborhood
  * stencil3d: structured 3D near neighbor pattern like that of jacobi/halo
  * stencil4d: structured 4D near neighbor pattern like that of jacobi/halo
  * subcom-a2a: FFT-style subcommunicator-based all to all communication
  * unstr-mesh: unstructured mesh communication pattern

### OTF2 Tracing

For OTF2 tracing, see notes here: https://github.com/LLNL/tracer/blob/master/README.OTF


### Reference

Any published work that utilizes this software should include the following
reference:

```
Nikhil Jain and Abhinav Bhatele. Chatterbug communication proxy
applications suite. https://github.com/LLNL/chatterbug. LLNL-CODE-756471.
```

### Release

Copyright (c) 2018, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.

Written by:
```
     Nikhil Jain <nikhil.jain@acm.org>
     Abhinav Bhatele <bhatele@llnl.gov>
```
LLNL-CODE-756471. All rights reserved.

This file is part of Chatterbug. For details, see:
https://github.com/LLNL/chatterbug.
Please also read the LICENSE file for the MIT License notice.
