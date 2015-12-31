#if USE_SHALLOC
#include "mpi.h"
#include "cktiming.h"
extern "C" void changeMessage(int size)
{
  BgTimeLine &timeLine = tTIMELINEREC.timeline;  
  timeLine[timeLine.length() - 3]->msgs[0]->msgsize = size;
}
#endif


