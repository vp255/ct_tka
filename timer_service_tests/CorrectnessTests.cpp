#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "timer_service/TimerService.h"
#include "timer_service_tests/GetSystemClockTime.h"
#include "timer_service_tests/CorrectnessTests.h"

namespace {

struct CallBackInfo {
  int id;
  int64_t time;
  void* context;

  bool operator<(const CallBackInfo& other) const {
    if (time == other.time)
      return id < other.id;
    return time < other.time;
  }
};

std::ostream& operator<<(std::ostream& oss, const CallBackInfo& obj) {
  oss << "id: " << obj.id
      << ", time: " << obj.time
      << ", context: " << obj.context;
  return oss;
}

struct RepeatingTimerState {
  unsigned count = 0;
  unsigned reps_until_cancel;
};

class TestingListener : public TimerService::IListener {

public:

  TestingListener(TimerService *timer_svc) :
    timer_svc_(timer_svc)
  { }

  void onTimerTriggered(int timer_id, void* user_ctx) override {
    executed_callbacks_.push_back(CallBackInfo{timer_id, GetSystemClockTime(), user_ctx});
    if (user_ctx != nullptr) {
      auto state = reinterpret_cast<RepeatingTimerState*>(user_ctx);
      if (++(state->count) == state->reps_until_cancel) {
        timer_svc_->cancelTimer(timer_id);
      }
    }
  }

  int setTimer(int64_t delay, void* context) {
    auto current_time = GetSystemClockTime();
    int64_t execution_time = current_time + delay;
    int timer_id = timer_svc_->setTimer(delay, this, context);
    expected_callbacks_.insert(CallBackInfo{timer_id, execution_time, context});
    return timer_id;
  }

  int setRepeatingTimer(int64_t delay, unsigned reps) {
    repeating_timer_states_.push_back(std::make_unique<RepeatingTimerState>(RepeatingTimerState{0, reps}));
    auto time = GetSystemClockTime();
    void* context = repeating_timer_states_.back().get();
    int timer_id = timer_svc_->setRepeatingTimer(delay, this, context);
    while (reps--) {
      time += delay;
      expected_callbacks_.insert(CallBackInfo{timer_id,time,context});
    }
    return timer_id;
  }

  void cancelTimer(int id) {
    timer_svc_->cancelTimer(id);
    auto it = expected_callbacks_.begin();
    while (it != expected_callbacks_.end()) {
      auto next = std::next(it, 1);
      if (it->id == id) {
        expected_callbacks_.erase(it);
      }
      it = next;
    }
  }

  void clearCallbacks() {
    executed_callbacks_.clear();
    expected_callbacks_.clear();
    repeating_timer_states_.clear();
  }

  void doneTime(int64_t time) {
    done_time = time;
  }

  void setDoneTimeInFromNow(int64_t delay) {
    done_time = delay + GetSystemClockTime();
  }

  int64_t doneTime() {
    return done_time;
  }

  bool verifyExpectedMeetsExecuted() {
    auto callTime = GetSystemClockTime();

    auto it = expected_callbacks_.begin(); 
    bool willreturn = true;
    for (decltype(executed_callbacks_.size()) i=0; i < executed_callbacks_.size(); i++) {
      if (it == expected_callbacks_.end()) {
        std::cout << "ran out of expected calls, executed " << i << '\n';
        return false;
      }
      const auto& executed = executed_callbacks_[i];
      const auto& expected = *it;

      if (executed.id != expected.id) {
        std::cout << "id mismatch, expected: " << expected.id << ", executed: " << executed.id << '\n';
        willreturn = false;
        //return false;
      }
      if (executed.context != expected.context) {
        std::cout << "context mismatch, expected: " << expected.context << ", executed: " << executed.context << '\n';
        willreturn = false;
        //return false;
      }
      auto time_diff = abs(executed.time - expected.time);
      if (time_diff > 150000) {
        std::cout << "time diff too big: " << time_diff << ", i: " << i << std::endl;
        willreturn = false;
      }
      it = std::next(it, 1);
    }

    if (!willreturn) {
      std::cout << "expected: " << '\n';
      for (auto e : expected_callbacks_)
        std::cout << "  " <<  e << '\n';

      std::cout << "executed: " << '\n';
      for (auto e : executed_callbacks_)
        std::cout << "  " <<  e << '\n';
    }

    return willreturn && (it == expected_callbacks_.end() || it->time < callTime);
  }

private:

  int64_t done_time = 0;
  TimerService* const timer_svc_;
  std::vector<CallBackInfo> executed_callbacks_{};
  std::set<CallBackInfo> expected_callbacks_{};
  std::vector<std::unique_ptr<RepeatingTimerState>> repeating_timer_states_{};
};

} // end of anonymous namespace

namespace timer_service_tests {

void SetTimerTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

  int64_t total_delay = 3 * 1000000000L;

  listener.setDoneTimeInFromNow(total_delay);
  listener.setTimer(total_delay, nullptr);

  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
}

void SparseTimerTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);
  
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
}

void InterleavingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);
  
  int64_t second = 1000000000L;

  listener.setRepeatingTimer(1 * second, 10);
  listener.setRepeatingTimer(2 * second, 6);
  listener.setDoneTimeInFromNow(13 * second);

  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }
  assert(listener.verifyExpectedMeetsExecuted());

}

void CancelingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

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
}

void NotCancelingTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);
  
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
}

void InvalidCancelTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

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
}

void CancelExpiredTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

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
}

void InOrderTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

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

}

void ModifyContextTest() {
  TimerService timer_svc(GetSystemClockTime());
  TestingListener listener(&timer_svc);

  int64_t total_delay = 3 * 1000000000L;

  RepeatingTimerState state{1, 0};

  listener.setDoneTimeInFromNow(total_delay);
  listener.setTimer(total_delay, &state);


  for (auto time = GetSystemClockTime(); time <= listener.doneTime(); time = GetSystemClockTime()) {
    timer_svc.advanceClock(time);
  }

  assert(listener.verifyExpectedMeetsExecuted());
  assert(state.count == 2);
}

void RunCorrectnessTests() {
  SetTimerTest();
  SparseTimerTest();
  InterleavingTest();
  CancelingTest();
  NotCancelingTest();
  InvalidCancelTest();
  CancelExpiredTest();
  InOrderTest();
  ModifyContextTest();
}

} // end of timer_service_tests namespace

