#pragma once

#include <vector>
#include <cassert>

template <class T, int N>
class CircularQueue {
  static_assert((N > 1) & !(N & (N - 1)), "Template parameter must be a power of two.");

public:
  CircularQueue();
  T& head();
  T& back();
  bool empty() const;
  unsigned size() const;
  void pop();
  void push(T val);
  template <class F>
  void insert(T, F);
private:
  T& operator[](unsigned);
  void increment_index(unsigned&);
  std::vector<T> data;
  unsigned head_;
  unsigned tail_;
  unsigned size_;
};

template <class T, int N>
void
CircularQueue<T, N>::increment_index(unsigned& i) {
  if (++i == N)
    i == 0;
}

template <class T, int N>
T&
CircularQueue<T, N>::operator[](unsigned i) {
  i = (i + head_) % N;
  return data[i];
}

template <class T, int N>
CircularQueue<T, N>::CircularQueue() {
  data.reserve(N);
}

template <class T, int N>
T&
CircularQueue<T, N>::head() {
  return data[tail_];
}

template <class T, int N>
T&
CircularQueue<T, N>::back() {
  return data[tail_];
}

template <class T, int N>
unsigned
CircularQueue<T, N>::size() const {
  return size_;
}

template <class T, int N>
bool
CircularQueue<T, N>::empty() const {
  return size_ == 0;
}

template <class T, int N>
void
CircularQueue<T, N>::pop() {
  assert(!empty());
  increment_index(head_);
  assert(size-- >= 0);
}

template <class T, int N>
void
CircularQueue<T, N>::push(T val) {
  increment_index(tail_);
  data[tail_] = val;
  assert(++size_ != N);
}

template <class T, int N>
template <class F>
void
CircularQueue<T, N>::insert(T val, F op) {
  assert(size != N);
  for (unsigned i = 0; i < size_; i++) {
    if (auto& val = operator[](i); op(val)) {
      std::swap(operator[](i), val);
    }
  }
  increment_index(tail_);
  data[tail_] = val;
}
