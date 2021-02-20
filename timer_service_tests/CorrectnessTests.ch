#include <cassert>
#include <iostream>

#include "timer_service_tests/GetSystemClockTime.h"
#include "timer_service_tests/CorrectnessTests.h"
#include "timer_service_tests/TestingListener.h"

namespace timer_service_tests {

template <class TimerService>
void SetTimerTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t total_delay = 3 * 1000000000L;

  listener.setDoneTimeInFromNow(total_delay);
  listener.setTimer(total_delay, nullptr);

  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed SetTimerTest" << std::endl;
}

template <class TimerService>
void SparseTimerTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);
  
  int64_t second = 1000000000L;
  int64_t long_delay = 10 * second;
  int64_t short_delay = 1 * second;
  int64_t medium_delay =  3 * second;

  listener.setTimer(long_delay, nullptr);
  listener.setTimer(short_delay, nullptr);
  listener.setTimer(medium_delay, nullptr);
  listener.setDoneTimeInFromNow(long_delay);

  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed SparseTimerTest" << std::endl;
}

template <class TimerService>
void InterleavingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);
  
  int64_t second = 1000000000L;

  listener.setRepeatingTimer(1 * second, 10);
  listener.setRepeatingTimer(2 * second, 6);
  listener.setDoneTimeInFromNow(13 * second);

  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed Interleaving Test" << std::endl;
}

template <class TimerService>
void CancelingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t second = 1000000000L;
  int64_t total_delay = 3 * second;


  bool canceled = false;
  auto time = GetSystemClockTime();
  bool cancel_time = time + 1 * second;

  listener.setDoneTimeInFromNow(total_delay);
  auto id = listener.setTimer(2 * second, nullptr);

  for (; time <= listener.doneTime(); time = GetSystemClockTime()) {
    if (!canceled && time > cancel_time) {
      listener.cancelTimer(id);
      canceled = true;
    }
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed Canceling Test" << std::endl;
}

template <class TimerService>
void NotCancelingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);
  
  int64_t second = 1000000000L;

  bool canceled = false;
  auto time = GetSystemClockTime();
  bool cancel_time = time + 4 * second;

  listener.setRepeatingTimer(1 * second, 10);
  int two_second_repeater = listener.setRepeatingTimer(2 * second, 6);
  listener.setDoneTimeInFromNow(13 * second);

  for (; time <= listener.doneTime(); time = GetSystemClockTime()) {
    if (!canceled && time > cancel_time) {
      listener.cancelTimer(two_second_repeater);
      canceled = true;
    }
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed NotCanceling Test" << std::endl;
}

template <class TimerService>
void InvalidCancelTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t second = 1000000000L;
  int64_t total_delay = 5 * second;

  listener.setDoneTimeInFromNow(total_delay);
  listener.setRepeatingTimer(second, 4);

  bool cancel_sent = false;
  auto time = GetSystemClockTime();
  bool cancel_time = time + 1 * second;

  for (; time <= listener.doneTime(); time = GetSystemClockTime()) {
    if (!cancel_sent && time > cancel_time ) {
      listener.cancelTimer(50);
      cancel_sent = true;
    }
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed InvalidCancel Test" << std::endl;
}

template <class TimerService>
void CancelExpiredTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t second = 1000000000L;
  int64_t total_delay = 5 * second;

  listener.setDoneTimeInFromNow(total_delay);
  int id = listener.setRepeatingTimer(second, 3);

  bool cancel_sent = false;
  auto time = GetSystemClockTime();
  bool cancel_time = time + 4 * second;

  for (; time <= listener.doneTime(); time = GetSystemClockTime()) {
    if (!cancel_sent && time > cancel_time ) {
      listener.cancelTimer(id);
      cancel_sent = true;
    }
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed CancelExpired Test" << std::endl;
}

template <class TimerService>
void InOrderTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t second = 1000000000L;
  int64_t total_delay = 11 * second;
  listener.setDoneTimeInFromNow(total_delay);

  listener.setTimer(2 * second, nullptr);
  listener.setTimer(second, nullptr);
  listener.setRepeatingTimer(second, 10);
  listener.setTimer(second, nullptr);

  bool more_timers_sent = false;
  auto time = GetSystemClockTime();
  bool more_timers_sent_time = time + 4 * second;

  for (; time <= listener.doneTime(); time = GetSystemClockTime()) {
    if (!more_timers_sent && time > more_timers_sent_time) {
      listener.setTimer(2 * second, nullptr);
      listener.setTimer(second, nullptr);
      listener.setRepeatingTimer(second, 10);
      listener.setTimer(second, nullptr);
      more_timers_sent = true;
    }
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());
  std::cout << "Passed InOrder Test" << std::endl;
}

template <class TimerService>
void ModifyContextTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener<TimerService> listener(&timer_svc);

  int64_t total_delay = 3 * 1000000000L;

  RepeatingTimerState state{1, 0};

  listener.setDoneTimeInFromNow(total_delay);
  listener.setTimer(total_delay, &state);


  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
  assert(state.count == 2);
  std::cout << "Passed ModifyContext Test" << std::endl;
}

template <class TimerService>
void RunCorrectnessTests() {
  SetTimerTest<TimerService>();
  SparseTimerTest<TimerService>();
  InterleavingTest<TimerService>();
  CancelingTest<TimerService>();
  NotCancelingTest<TimerService>();
  InvalidCancelTest<TimerService>();
  CancelExpiredTest<TimerService>();
  InOrderTest<TimerService>();
  ModifyContextTest<TimerService>();
}

} // end of timer_service_tests namespace
