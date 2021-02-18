#pragma once
#include <set>
#include "timer_service/CircularQueue.h"

template <class Listener>
struct TimerQueueElement {
  int id;
  int64_t alert_time;
  Listener* listener;
  void* context = nullptr;
  uint64_t interval;

  bool operator<(const TimerQueueElement<Listener>& other) const {
    if (alert_time == other.alert_time)
      return id < other.id;
    else
      return alert_time < other.alert_time;
  }
};

template <class Listener>
TimerQueueElement<Listener>
GetNextTimerQueueElement(TimerQueueElement<Listener> element) {
  element.alert_time += element.interval;
  return element;
}

template <class Listener>
class TimerQueue {
public:
  typedef TimerQueueElement<Listener> value_type;
public:
  void push(value_type);
  void pop();
  TimerQueueElement<Listener>& top();
  void erase(int id);
  bool empty() const;
  uint16_t size() const;

  value_type& head();
  value_type& back();
  TimerQueue();

private:

  void updateFirstRepeatedElement();
  CircularQueue<value_type, 1024> queue;
  std::set<value_type> repeated_timers;
};

template <class Listener>
TimerQueue<Listener>::TimerQueue() {
}

template <class Listener>
bool
TimerQueue<Listener>::empty() const {
  // TODO
  return queue.empty();
}

template <class Listener>
void
TimerQueue<Listener>::erase(int) {
  // TODO
}

template <class Listener>
void
TimerQueue<Listener>::updateFirstRepeatedElement() {
  auto it = repeated_timers.begin();
  auto next = GetNextTimerQueueElement(*it);
  repeated_timers.erase(it);
  repeated_timers.insert(next);
}

template <class Listener>
void TimerQueue<Listener>::push(TimerQueueElement<Listener> element) {
  if (empty()) {
    if (!repeated_timers.empty()) {

      while (*repeated_timers.begin() < element) {
        queue.push(*repeated_timers.begin());
        updateFirstRepeatedElement();
      }
    }
    queue.push(element);
  }
  queue.insert(element, [&](auto e) {
      return element.alert_time > e.alert_time;
      });
}

template <class Listener>
void TimerQueue<Listener>::pop() {
  bool queue_empty = queue.empty();
  bool repeated_timers_empty = repeated_timers.empty();

  if (queue_empty && repeated_timers.empty())
    return;

  if (repeated_timers_empty || !queue_empty && *repeated_timers.begin() > queue.head) {
    auto& ele = queue.head();
    ele.listener.onTimerTriggered(ele.id, ele.context);
    queue.pop();
  } else {
    auto& ele = *repeated_timers.begin();
    ele.listener.onTimerTriggered(ele.id, ele.context);
    updateFirstRepeatedElement();
  }
}

template <class Listener>
typename TimerQueue<Listener>::value_type&
TimerQueue<Listener>::top() {
  if (queue.empty())
    return *repeated_timers.begin();
  else if (repeated_timers.begin())
    return queue.head();
  else {
    if (queue.head() < *repeated_timers.begin) {
      return queue.head();
    } else
      return *repeated_timers.begin();
  }
}
