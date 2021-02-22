#pragma once

#include <sstream>

template <class Listener>
struct TimerServiceElement {
  int64_t alert_time = 0;
  int64_t interval = 0;
  Listener* listener = nullptr;
  void* context = nullptr;
  int id = -1;
  bool invalid = true;

  bool operator<(const TimerServiceElement<Listener>& other) const {
    if (alert_time == other.alert_time)
      return id < other.id;
    else
      return alert_time < other.alert_time;
  }

  bool operator>(const TimerServiceElement<Listener>& other) const {
    if (alert_time == other.alert_time)
      return id > other.id;
    else
      return alert_time > other.alert_time;
  }

  bool operator==(const TimerServiceElement<Listener>& other) const {
    return !(*this < other) && !(*this > other);
  }
};

template <class Listener>
std::ostream& operator<<(std::ostream& oss, const TimerServiceElement<Listener>& element) {
  oss << " id: " << element.id
      << " interval: " << element.interval
      << " alert_time: " << element.alert_time
      << " context_ptr: " << element.context
      << " listener_ptr: " << element.listener;
  return oss;
}
