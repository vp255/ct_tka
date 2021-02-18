#include "timer_service/TimerService.h"
#include <iostream>

TimerService::TimerService(epoch_ns_t current_time_ns) : current_time(current_time_ns) {}

TimerService::~TimerService() {}

int TimerService::setTimer(int64_t alert_after_elapsed_ns, IListener* listener, void* user_ctx) {
  int id = counter++;
  queue.push(TimerServiceElement<IListener>{current_time + alert_after_elapsed_ns, 0, listener, user_ctx, id});
  return id;
}

int TimerService::setRepeatingTimer(int64_t interval, IListener* listener, void* user_ctx) {
  int id = counter++;
  queue.push(TimerServiceElement<IListener>{current_time + interval, interval, listener, user_ctx, id});
  return id;
}

void TimerService::cancelTimer(int timer_id) {
  if (timer_id < counter) {
    canceled_timers.insert(timer_id);
  }
}

void TimerService::advanceClock(epoch_ns_t current_time_ns) {
  current_time = current_time_ns;
  while (!queue.empty() && queue.top().alert_time <= current_time) {
    auto element = queue.top();
    queue.pop();
    if (canceled_timers.count(element.id)) {
      canceled_timers.erase(element.id);
    } else {
      element.listener->onTimerTriggered(element.id, element.context);
      if (element.interval) {
        queue.push(TimerServiceElement<IListener>{element.alert_time + element.interval,
                                                  element.interval,
                                                  element.listener,
                                                  element.context,
                                                  element.id});
      }
    }
  }
  if (queue.empty()) {
    canceled_timers.clear();
  }
}
