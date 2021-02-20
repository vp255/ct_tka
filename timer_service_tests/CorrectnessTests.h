#pragma once

namespace timer_service_tests {

template <class TimerService>
void SetTimerTest();

template <class TimerService>
void SparseTimer();

template <class TimerService>
void InterleavingTest();

template <class TimerService>
void CancelingTest();

template <class TimerService>
void NotCancelingTest();

template <class TimerService>
void InvalidCancelTest();

template <class TimerService>
void CancelExpiredTest();

template <class TimerService>
void InOrderTest();

template <class TimerService>
void ModifyContextTest();

template <class TimerService>
void RunCorrectnessTests();

}

#include "timer_service_tests/CorrectnessTests.ch"
