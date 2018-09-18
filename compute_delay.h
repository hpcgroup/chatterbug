#include <errno.h>
#include <time.h>

void fake_compute(long long computeTime) {
  struct timespec sleepTime, remainingTime;
  sleepTime.tv_sec = computeTime/1000000000;
  sleepTime.tv_nsec = computeTime - (sleepTime.tv_sec * 1000000000);

  if (nanosleep(&sleepTime, &remainingTime) == EINTR) {
    while (nanosleep(&remainingTime, &remainingTime) == EINTR);
  }
}
