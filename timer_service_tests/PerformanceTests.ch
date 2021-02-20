#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include "timer_service_tests/GetSystemClockTime.h"
#include "timer_service_tests/PerformanceTests.h"
#include "timer_service_tests/TestingListener.h"

namespace {

//void InitializeStartup(TimerService& timer_service, timer_service_tests::TestingListener& listener, int64_t runtime_ns) {
//
//  int64_t second = 1000000000L;
//  
//  listener.setDoneTimeInFromNow(runtime_ns);
//  listener.setRepeatingTimer(second / 100);
//  listener.setRepeatingTimer(second / 10);
//  listener.setRepeatingTimer(second);
//  listener.setRepeatingTimer(10 * second);
//}


template<class D>
auto MeasureFunction(std::function<void()> f) {
  auto t1 = std::chrono::high_resolution_clock::now();
  f();
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<D>( t2 - t1 ).count();
}

template <class TimerService>
auto setNRandomTimerWithTDelay(timer_service_tests::TestingListener<TimerService>& listener, int16_t timers, int64_t delay_between_timers) {
  int64_t delay = 0;

  std::vector<int64_t> delays;
  for (int16_t i=0; i < timers; i++) {
    delay += delay_between_timers;
    listener.setTimer(delay, nullptr);
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(delays.begin(), delays.end(), g);
  return [delays = std::move(delays), &listener]() {
    for (const auto& d : delays) {
      listener.setTimer(d, nullptr);
    }
  };

}

} // end of anonymous namespace

namespace timer_service_tests {

  template <class TimerService>
  void AdvanceClockTest(const std::vector<int64_t>& repeating_timer_delays, int16_t timers, int64_t delay_between_timers) {
    TimerService timer_svc(GetSystemClockTime());
    TestingListener<TimerService> listener(&timer_svc);

    int64_t second = 1000000000L;
    
    int64_t done_time = timers * delay_between_timers;
    listener.setDoneTimeInFromNow(done_time);

    for (const auto& delay : repeating_timer_delays) {
      listener.setRepeatingTimer(delay);
    }

    std::cout << "setting up " << timers << " timers "
              << "over " << done_time << " nanoseconds "
              << "takes " << MeasureFunction<std::chrono::nanoseconds>(setNRandomTimerWithTDelay(listener, 10000, 100 * second))
              << " nanoseconds "
              << '\n';

    int64_t times_entered_scope = 0;

    auto l1_cache = std::make_unique<std::vector<int8_t>>(1<<16);

    for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
      for (auto& e : *l1_cache) ++e; // flush out l1 cache
      ++times_entered_scope;
      timer_svc.advanceClock(time);
    }

    std::cout << "times_entered_scope: " << times_entered_scope << std::endl;
  }

  template <class TimerService>
  void RunPerformanceTests() {
    int64_t second = 1000000000L;
    AdvanceClockTest<TimerService>({second / 100, second / 10}, 1000, second / 100);
  }

} // end of timer_service_tests namespace
