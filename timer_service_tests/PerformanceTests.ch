#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include "timer_service_tests/GetSystemClockTime.h"
#include "timer_service_tests/PerformanceTests.h"
#include "timer_service_tests/TestingListener.h"

namespace {

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
  void AdvanceClockTest(const std::vector<int64_t>& repeating_timer_delays, int16_t timers, int64_t delay_between_timers, bool flush_cache) {

    int64_t second = 1000000000L;
    int64_t done_time = timers * delay_between_timers;
    std::cout << "    Executing " << timers << " timers test "
              << "over " << done_time << " (ns), or " << (done_time/second) << " (s) with 1 second of processing with no timers WITH";

    if (!flush_cache) {
      std::cout << "OUT";
    }
    std::cout << " cache flushes: " << '\n';
    done_time += second;

    TimerService timer_svc(GetSystemClockTime());
    TestingListener<TimerService> listener(&timer_svc);

    listener.setDoneTimeInFromNow(done_time);

    std::cout << "           "
              << "setting up " << repeating_timer_delays.size() << " repeating_timers of sizes: ";
    for (const auto& delay : repeating_timer_delays) {
      std::cout << delay << " (ns), ";
    }


    std::cout << " which takes " << MeasureFunction<std::chrono::nanoseconds>([&]() {
                                                                                for (const auto& delay : repeating_timer_delays) {
                                                                                  listener.setRepeatingTimer(delay);
                                                                                }
                                                                             })
              << " nanoseconds "
              << '\n';


    std::cout << "           "
              << "setting up " << timers << " timers "
              << "over " << done_time << " nanoseconds "
              << "takes " << MeasureFunction<std::chrono::nanoseconds>(setNRandomTimerWithTDelay(listener, 1000, timers * delay_between_timers))
              << " nanoseconds "
              << '\n';

    int64_t times_entered_scope = 0;

    std::vector<int8_t> l1_cache(1<<16);

    for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
      if (flush_cache) {
        for (auto& e : l1_cache) ++e; // flush out l1 cache
      }
      ++times_entered_scope;
      timer_svc.advanceClock(time);
    }

    std::cout << "           "
              << "RESULTS: times_entered_scope: " << times_entered_scope << " over " << done_time << " (ns), or " << (done_time/second) << " (s)\n";
  }

  template <class TimerService>
  void RunPerformanceTests() {
    int64_t second = 1000000000L;
    AdvanceClockTest<TimerService>({second / 100, second / 10, second}, 1000, second / 100, false);
    AdvanceClockTest<TimerService>({second / 100, second / 10, second}, 1000, second / 100, true);
    AdvanceClockTest<TimerService>({}, 1000, second / 100, false);
    AdvanceClockTest<TimerService>({}, 1000, second / 100, true);
    AdvanceClockTest<TimerService>({second / 100, second / 10, second}, 1000, second / 10, false);
    AdvanceClockTest<TimerService>({second / 100, second / 10, second}, 1000, second / 10, true);
    AdvanceClockTest<TimerService>({}, 1000, second / 10, false);
    AdvanceClockTest<TimerService>({}, 1000, second / 10, true);
  }

} // end of timer_service_tests namespace
