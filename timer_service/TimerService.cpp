#include "timer_service/TimerService.h"
#include "timer_service/TimerQueue.h"
#include <iostream> // REMOVE LATER


TimerService::TimerService(epoch_ns_t current_time_ns) : current_time(current_time_ns) {}

TimerService::~TimerService() {}

int TimerService::setTimer(int64_t alert_after_elapsed_ns, IListener* listener, void* user_ctx) {
  int id = counter++;
  timer_queue.push(TimerQueueElement<IListener>{counter, current_time + alert_after_elapsed_ns, listener, user_ctx, 0});
  return id;
}

int TimerService::setRepeatingTimer(int64_t interval, IListener* listener, void* user_ctx) {
  std::cout << interval << listener << user_ctx << std::endl;
  return 0;
}

void TimerService::cancelTimer(int timer_id) {
  std::cout << timer_id << std::endl;

}

void TimerService::advanceClock(epoch_ns_t current_time_ns) {
  std::cout << current_time_ns << std::endl;

}
