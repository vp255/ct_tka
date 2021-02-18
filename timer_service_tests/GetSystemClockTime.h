#pragma once
#include <iostream>

inline int64_t GetSystemClockTime() {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    std::cerr << "clock_gettime() failed\n";
    abort();
  }

  return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

