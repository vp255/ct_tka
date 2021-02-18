#pragma once

#include "timer_service/TimerService.h"
#include "timer_service_tests/GetSystemClockTime.h"
#include "timer_service_tests/CorrectnessTests.h"

#include <iostream>
#include <cassert>

struct DemoListener : public TimerService::IListener {
  TimerService* const timer_svc_;
  bool done_;
  int timer_id_a_;
  int timer_id_b_;
  int timer_id_c_;
  int a_trigger_count_;
  int b_trigger_count_;
  int c_trigger_count_;

  DemoListener(TimerService *timer_svc) :
    timer_svc_(timer_svc),
    done_(false),
    timer_id_a_(TimerService::NO_TIMER_ID),
    timer_id_b_(TimerService::NO_TIMER_ID),
    timer_id_c_(TimerService::NO_TIMER_ID),
    a_trigger_count_(0),
    b_trigger_count_(0),
    c_trigger_count_(0)
  { }

  void onTimerTriggered(int timer_id, void* user_ctx) override {
    assert(user_ctx == nullptr);

    if (timer_id == timer_id_a_) {
      a_trigger_count_++;
      timer_id_a_ = TimerService::NO_TIMER_ID;
      std::cout << "One-shot timer A triggered\n";
      timer_id_b_ = timer_svc_->setRepeatingTimer(10 * 1000000000L, this, nullptr);
      timer_id_c_ = timer_svc_->setRepeatingTimer(5 * 1000000000L, this, nullptr);
    } else if (timer_id == timer_id_b_) {
      std::cout << "Periodic timer B triggered\n";
      if (++b_trigger_count_ == 10) {
        timer_svc_->cancelTimer(timer_id_b_);
        timer_id_b_ = TimerService::NO_TIMER_ID;
      }
    } else {
      assert(timer_id == timer_id_c_);
      std::cout << "Periodic timer C triggered\n";
      if (++c_trigger_count_ == 10) {
        timer_svc_->cancelTimer(timer_id_c_);
        timer_id_c_ = TimerService::NO_TIMER_ID;
      }
    }

    if ((timer_id_a_ == TimerService::NO_TIMER_ID) &&
        (timer_id_b_ == TimerService::NO_TIMER_ID) &&
        (timer_id_c_ == TimerService::NO_TIMER_ID)) {
      std::cout << "Done\n";
      done_ = true;
    }
  }
};

void RunDemoListenerTests() {
  TimerService timer_svc(GetSystemClockTime());

  DemoListener listener(&timer_svc);

  listener.timer_id_a_ = timer_svc.setTimer(3 * 1000000000L, &listener, nullptr);

  while (!listener.done_) {
    timer_svc.advanceClock(GetSystemClockTime());
  }

  assert(listener.a_trigger_count_ == 1);
  assert(listener.b_trigger_count_ == 10);
  assert(listener.c_trigger_count_ == 10);
}
