#pragma once

template <class Listener>
struct TimerServiceElement {
  int64_t alert_time;
  int64_t interval = 0;
  Listener* listener = nullptr;
  void* context = nullptr;
  int id;

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
};

