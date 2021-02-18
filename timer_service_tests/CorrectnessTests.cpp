#include <iostream>
#include <map>
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

  void setRepeatingTimer(int64_t delay, unsigned reps) {
    repeating_timer_states_.push_back(RepeatingTimerState{0, reps});
    auto time = GetSystemClockTime();
    void* context = &repeating_timer_states_.back();
    int timer_id = timer_svc_->setRepeatingTimer(delay, this, context);
    while (reps--) {
      time += delay;
      expected_callbacks_.insert(CallBackInfo{timer_id,time,context});
    }
  }

  void cancelTimer(int id) {
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

  void printCallBackInfos() {
    std::cout << "Expected: " << '\n';
    for (auto e : expected_callbacks_) {
      std::cout << e << '\n';
    }
    std::cout << "Executed: " << '\n';
    for (auto e : executed_callbacks_) {
      std::cout << e << '\n';
    }
  }

private:

  int64_t done_time = 0;
  TimerService* const timer_svc_;
  std::vector<CallBackInfo> executed_callbacks_{};
  std::set<CallBackInfo> expected_callbacks_{};
  std::vector<RepeatingTimerState> repeating_timer_states_{};
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

  listener.printCallBackInfos();
}

void CancelingTest() {
}

void InvalidCancelTest() {

}

void CancelExpiredTest() {

}

void InOrderTest() {

}

void SetRepeatingTimerTest() {

}

void RunCorrectnessTests() {
  SetTimerTest();
  CancelingTest();
  InvalidCancelTest();
  CancelExpiredTest();
  InOrderTest();
  SetRepeatingTimerTest();
}

} // end of timer_service_tests

