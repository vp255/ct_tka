#include "timer_service/TimerService.h"

TimerService::TimerService(epoch_ns_t current_time_ns) : current_time(current_time_ns) {}

TimerService::~TimerService() {}

int TimerService::setTimer(int64_t alert_after_elapsed_ns, IListener* listener, void* user_ctx) {
  int id = counter++;
  flat_set.insert(TimerServiceElement<IListener>{current_time + alert_after_elapsed_ns, 0, listener, user_ctx, id, false});
  return id;
}

int TimerService::setRepeatingTimer(int64_t interval, IListener* listener, void* user_ctx) {
  int id = counter++;
  flat_set.insert(TimerServiceElement<IListener>{current_time + interval, interval, listener, user_ctx, id, false});
  return id;
}

void TimerService::cancelTimer(int timer_id) {
  flat_set.invalidate(timer_id);
}

void TimerService::advanceClock(epoch_ns_t current_time_ns) {
  while (!flat_set.empty() && flat_set.head().alert_time <= current_time_ns) {
    auto& element = flat_set.head();
    if (!element.invalid) {
      element.listener->onTimerTriggered(element.id, element.context);
      if (element.interval && !element.invalid) {
        flat_set.insert(TimerServiceElement<IListener>{element.alert_time + element.interval,
                                                       element.interval,
                                                       element.listener,
                                                       element.context,
                                                       element.id,
                                                       false});
      }
    }
    flat_set.pop();
  }
}
