#pragma once

#include <memory>
#include <set>
#include <vector>
#include "timer_service_tests/GetSystemClockTime.h"

namespace timer_service_tests {

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

inline std::ostream& operator<<(std::ostream& oss, const CallBackInfo& obj) {
  oss << "id: " << obj.id
      << ", time: " << obj.time
      << ", context: " << obj.context;
  return oss;
}

struct RepeatingTimerState {
  unsigned count = 0;
  unsigned reps_until_cancel;
};

template <class TimerService>
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

  int setRepeatingTimer(int64_t delay) {
    return timer_svc_->setRepeatingTimer(delay, this, nullptr);
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
      }
      if (executed.context != expected.context) {
        std::cout << "context mismatch, expected: " << expected.context << ", executed: " << executed.context << '\n';
        willreturn = false;
      }
      auto time_diff = abs(executed.time - expected.time);
      if (time_diff > 1000000) { // adjust
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

} // end of timer_service_tests namespace
