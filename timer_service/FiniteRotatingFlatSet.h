#pragma once

#include <vector>
#include <cassert>

template <class T, int N>
class FiniteRotatingFlatSet {
public:
  FiniteRotatingFlatSet();
  const T& head() const;
  const T& back() const;
  bool empty() const;
  unsigned size() const;
  void pop();
  bool insert(T);
  bool erase(T);
  bool invalidate(int);
private:
  T& operator[](unsigned);
  const T& operator[](unsigned) const;
  void increment_index(unsigned&);
  void decrement_index(unsigned&);
  int rotated_lower_bound(const T&) const;
  void erase_at_index(unsigned);
  std::vector<T> data;
  unsigned head_ = 0;
  unsigned tail_ = N;
  unsigned size_ = 0;
};

template <class T, int N>
void
FiniteRotatingFlatSet<T, N>::increment_index(unsigned& i) {
  if (++i >= N)
    i -= N;
}

template <class T, int N>
void
FiniteRotatingFlatSet<T, N>::decrement_index(unsigned& i) {
  if (i-- == 0)
    i = N;
}

template <class T, int N>
T&
FiniteRotatingFlatSet<T, N>::operator[](unsigned i) {
  return const_cast<T&>(static_cast<const FiniteRotatingFlatSet<T, N>&>(*this).operator[](i));
}

template <class T, int N>
const T&
FiniteRotatingFlatSet<T, N>::operator[](unsigned i) const {
  i = (i + head_) % N;
  return data[i];
}

template <class T, int N>
FiniteRotatingFlatSet<T, N>::FiniteRotatingFlatSet() : data(N) { }

template <class T, int N>
const T&
FiniteRotatingFlatSet<T, N>::head() const {
  return data[head_];
}

template <class T, int N>
const T&
FiniteRotatingFlatSet<T, N>::back() const {
  return data[tail_];
}

template <class T, int N>
unsigned
FiniteRotatingFlatSet<T, N>::size() const {
  return size_;
}

template <class T, int N>
bool
FiniteRotatingFlatSet<T, N>::empty() const {
  return size_ == 0;
}

template <class T, int N>
void
FiniteRotatingFlatSet<T, N>::pop() {
  assert(!empty());
  increment_index(head_);
  size_--;
}

template <class T, int N>
int
FiniteRotatingFlatSet<T, N>::rotated_lower_bound(const T& val) const {
  int left = 0;
  int right = size_ - 1;

  while (left <= right) {
    int mid = left + (right - left) / 2;
    const auto& mid_val = operator[](mid);
    if (mid_val == val) {
      return mid;
    } else if (mid_val < val) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return left;
}

template <class T, int N>
bool
FiniteRotatingFlatSet<T, N>::insert(T val) {
  assert(size_ != N);

  auto index = rotated_lower_bound(val);
  if (operator[](index) == val) {
    return false;
  }

  for (unsigned i = index; i <= size_; i++) {
    std::swap(operator[](i), val);
  }
  increment_index(tail_);
  size_++;
  return true;
}

template <class T, int N>
bool
FiniteRotatingFlatSet<T, N>::erase(T val) {
  if (empty())
    return false;

  auto index = rotated_lower_bound(val);
  if (operator[](index) != val) {
    return false;
  }
  erase_at_index(index);
  return true;
}

template <class T, int N>
bool
FiniteRotatingFlatSet<T, N>::invalidate(int id) {
  if (empty())
    return false;

  for (unsigned i = 0; i < size_; i++) {
    if (operator[](i).id == id) {
      operator[](i).invalid = true;
      return true;
    }
  }
  return false;
}

template <class T, int N>
void
FiniteRotatingFlatSet<T, N>::erase_at_index(unsigned index) {
  if (index == size_ - 1) {
    operator[](index) = T{};
  } else {
    for (unsigned i = index; i < size_; i++) {
      std::swap(operator[](i), operator[](i+1));
    }
  }
  decrement_index(tail_);
  size_--;
}

