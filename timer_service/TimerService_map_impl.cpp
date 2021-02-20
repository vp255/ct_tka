#include "timer_service/TimerService_map_impl.h"
#include <iostream>

TimerService_map_impl::TimerService_map_impl(epoch_ns_t current_time_ns) : current_time(current_time_ns) {}

TimerService_map_impl::~TimerService_map_impl() {}

int TimerService_map_impl::setTimer(int64_t alert_after_elapsed_ns, IListener* listener, void* user_ctx) {
  int id = counter++;
  timers.insert(TimerServiceElement<IListener>{current_time + alert_after_elapsed_ns, 0, listener, user_ctx, id});
  return id;
}

int TimerService_map_impl::setRepeatingTimer(int64_t interval, IListener* listener, void* user_ctx) {
  int id = counter++;
  timers.insert(TimerServiceElement<IListener>{current_time + interval, interval, listener, user_ctx, id});
  return id;
}

void TimerService_map_impl::cancelTimer(int timer_id) {
  if (timer_id < counter) {
    canceled_timers.insert(timer_id);
  }
}

void TimerService_map_impl::advanceClock(epoch_ns_t current_time_ns) {
  current_time = current_time_ns;
  while (!timers.empty() && timers.begin()->alert_time <= current_time) {
    auto it = timers.begin();
    const auto& element = *it;
    if (canceled_timers.count(element.id)) {
      canceled_timers.erase(element.id);
    } else {
      element.listener->onTimerTriggered(element.id, element.context);
      if (element.interval) {
        timers.insert(TimerServiceElement<IListener>{element.alert_time + element.interval,
                                                  element.interval,
                                                  element.listener,
                                                  element.context,
                                                  element.id});
      }
    }
    timers.erase(it);
  }
  if (timers.empty()) {
    canceled_timers.clear();
  }
}
