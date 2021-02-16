#pragma once

#include <stdint.h>
#include "timer_service/TimerQueue.h"

// ----------------------------------- Global type definitions -------------------------------------
// System clock measurement, expressed as the number of nanoseconds elapsed since the UNIX Epoch.
typedef int64_t epoch_ns_t;


// ----------------------------- TimerService class declaration ----------------------------------
class TimerService final {
public:
  // TimerService event listener interface.
  class IListener {
  public:
    // Arguments:
    //   timer_id:  ID of the timer that triggered.
    //   user_ctx:  Opaque context provided by the client when creating the timer.
    virtual void onTimerTriggered(int timer_id, void* user_ctx) = 0;
  };


  // Constructor
  // Arguments:
  //   current_time_ns:  Current system time.
  TimerService(epoch_ns_t current_time_ns);

  // Destructor
  ~TimerService();

  // Register a one-shot timer that will be removed from the list of pending timers after it
  // triggers once.
  // Arguments:
  //   alert_after_elapsed_ns:  This timer will trigger when the system clock advances past
  //                            (current_time_ns + alert_after_elapsed_ns).
  //   listener:                Recipient of the timer callback.
  //   user_ctx:                Opaque context to be provided to the recipient.
  //
  // Return value:              ID of the newly-created timer.
  int setTimer(int64_t alert_after_elapsed_ns, IListener* listener, void* user_ctx);

  // Register a periodic timer that will be triggered at regular time intervals.
  // Arguments:
  //   alert_at_each_elapsed_ns:  Timer periodicity in nanoseconds. This timer will trigger
  //                              when the clock advances past (current_time_ns + alert_at_each_elapsed_ns),
  //                              and will repeat.
  //   listener:                  Recipient of the timer callback.
  //   user_ctx:                  Opaque context to be provided to the recipient.
  //
  // Return value:                ID of the newly-created timer.
  int setRepeatingTimer(int64_t alert_at_each_elapsed_ns, IListener* listener, void* user_ctx);

  // Remove a timer from the list of active timers.
  // Arguments:
  //   timer_id:      ID of the timer to remove. (Must match value returned from an earlier call
  //                  to setTimer/setRepeatingTimer). If timer_id does not refer to a valid active timer,
  //                  the behavior of this method is undefined.
  void cancelTimer(int timer_id);

  // Notify the timer service of changes to the value of the system clock. In this method, the timer
  // service must check if any timers triggered and notify the affected listeners via the
  // onTimerTriggered callback.
  // Arguments:
  //   current_time_ns:  Value of the most recent available system clock measurement.
  void advanceClock(epoch_ns_t current_time_ns);

  // Clients can use NO_TIMER_ID as a sentinel to indicate an invalid/uninitialized timer_id. This value
  // will never be returned from a call to setTimer/setRepeatingTimer.
  static constexpr int NO_TIMER_ID = -1;


  TimerService()                               = delete; // Default constructor intentionally disabled
  TimerService(const TimerService&)            = delete; // Copy construction intentionally disabled
  TimerService& operator=(const TimerService&) = delete; // Assignment operator intentionally disabled
private:
  TimerQueue<IListener> timer_queue;
  int counter = 0;
  epoch_ns_t current_time;
};
