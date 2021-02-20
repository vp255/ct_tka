#pragma once

namespace timer_service_tests {

template <class TimerService>
void AdvanceClockTest();

template <class TimerService>
void RunPerformanceTests();

}

#include "PerformanceTests.ch"
