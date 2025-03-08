#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_DATA_POOL_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_DATA_POOL_H_

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

namespace ecsify::internal {

// An iterator that adnvances to the next element based on the mask.
// If the bit is set, the iteration is skipped. Otherwise, the iterator advances
// as usual.
template <typename IteratorT, typename MaskT>
  requires(std::bidirectional_iterator<IteratorT>)
class MaskGuidedIterator {
 public:
  using Iterator = IteratorT;
  using Mask = MaskT;

  using iterator_category = std::forward_iterator_tag;
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  using difference_type =
      typename std::iterator_traits<Iterator>::difference_type;
  using pointer = typename std::iterator_traits<Iterator>::pointer;
  using reference = typename std::iterator_traits<Iterator>::reference;

  MaskGuidedIterator() : it_{}, mask_{} {};

  MaskGuidedIterator(Iterator iter, Mask mask) : it_(iter), mask_(mask) {
    SkipDisabled();
  }

  reference operator*() const {
    assert((mask_ & 1) == 0 && "Dereferencing disabled iterator");
    return *it_;
  }

  pointer operator->() const {
    assert((mask_ & 1) == 0 && "Dereferencing disabled iterator");
    return &(*it_);
  }

  MaskGuidedIterator &operator++() {
    assert((mask_ & 1) == 0 && "Incrementing disabled iterator");
    ++it_;
    mask_ >>= 1;
    SkipDisabled();
    return *this;
  }

  MaskGuidedIterator operator++(int) {
    assert((mask_ & 1) == 0 && "Incrementing disabled iterator");
    MaskGuidedIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  bool Equals(const MaskGuidedIterator &other) const {
    return it_ == other.it_;
  }

 private:
  void SkipDisabled() {
    std::size_t shift = std::countr_one(mask_);
    it_ += shift;
    mask_ >>= shift;
  }

  Iterator it_;
  Mask mask_;
};

template <class IterT, class MaskT>
bool operator==(const MaskGuidedIterator<IterT, MaskT> &lhs,
                const MaskGuidedIterator<IterT, MaskT> &rhs) {
  return lhs.Equals(rhs);
}

template <class IterT, class MaskT>
bool operator!=(const MaskGuidedIterator<IterT, MaskT> &lhs,
                const MaskGuidedIterator<IterT, MaskT> &rhs) {
  return !lhs.Equals(rhs);
}

template <class T>
  requires(std::default_initializable<T>)
const auto kDefaultCtor = []() { return T(); };

template <class T>
  requires(std::default_initializable<T> && std::copyable<T>)
class Bucket final {
 public:
  static consteval std::size_t Capacity() noexcept {
    return std::numeric_limits<Mask>::digits;
  }

  using Mask = std::uint64_t;
  using Iterator =
      MaskGuidedIterator<typename std::array<T, Capacity()>::iterator, Mask>;
  using ConstIterator =
      MaskGuidedIterator<typename std::array<T, Capacity()>::const_iterator,
                         Mask>;

  explicit Bucket(const T &val) { std::ranges::fill(data_, val); }

  std::size_t Insert(const T &val) noexcept(
      noexcept(std::declval<T &>() = std::declval<T &>())) {
    assert(!Full() && "Bucket is full");
    std::size_t offset = std::countr_zero(free_elements_mask_);
    data_[offset] = val;
    free_elements_mask_ &= ~GetMaskWithNthBitSet(offset);
    return offset;
  }

  std::size_t Insert(T &&val) noexcept {
    assert(!Full() && "Bucket is full");
    std::size_t offset = std::countr_zero(free_elements_mask_);
    data_[offset] = std::move(val);
    free_elements_mask_ &= ~GetMaskWithNthBitSet(offset);
    return offset;
  }

  void Erase(std::size_t idx) noexcept {
    assert(Contains(idx) && "Element doesn't exist");
    free_elements_mask_ |= GetMaskWithNthBitSet(idx);
  }

  T &operator[](std::size_t idx) noexcept {
    assert(Contains(idx) && "Element doesn't exist");
    return data_[idx];
  }

  const T &operator[](std::size_t idx) const noexcept {
    assert(Contains(idx) && "Element doesn't exist");
    return data_[idx];
  }

  bool Contains(std::size_t idx) const noexcept {
    assert(idx < Capacity() && "Index out of bounds");
    return (free_elements_mask_ & GetMaskWithNthBitSet(idx)) == 0;
  }

  bool Full() const noexcept { return free_elements_mask_ == 0; }

  Iterator begin() noexcept {
    return MaskGuidedIterator{data_.begin(), free_elements_mask_};
  }

  Iterator end() noexcept { return MaskGuidedIterator{data_.end(), Mask{}}; }

  ConstIterator begin() const noexcept {
    return MaskGuidedIterator{data_.begin(), free_elements_mask_};
  }

  ConstIterator end() const noexcept {
    return MaskGuidedIterator{data_.end(), Mask{}};
  }

 private:
  static Mask GetMaskWithNthBitSet(std::size_t n) noexcept {
    assert(n < Capacity() && "Shift number is too large");
    return static_cast<Mask>(1) << n;
  }

  std::array<T, Capacity()> data_{};
  // 1 means free, 0 means occupied.
  Mask free_elements_mask_ = std::numeric_limits<Mask>::max();
};

template <class T>
  requires(std::input_or_output_iterator<T>)
using IteratorValueType = typename std::iterator_traits<T>::value_type;

template <class T>
concept ContainerLike = requires(T c) {
  { std::begin(c) } -> std::input_or_output_iterator;
  { std::end(c) } -> std::input_or_output_iterator;
};

template <class T>
  requires(ContainerLike<T>)
using ContainerIteratorType = decltype(std::begin(std::declval<T>()));

template <class OuterIterT>
  requires(std::forward_iterator<OuterIterT> &&
           ContainerLike<IteratorValueType<OuterIterT>> &&
           std::forward_iterator<
               ContainerIteratorType<IteratorValueType<OuterIterT>>>)
class FlattenedIterator {
 public:
  using InnerIterT = ContainerIteratorType<typename std::iterator_traits<OuterIterT>::reference>;
  using iterator_category = std::forward_iterator_tag;
  using value_type = IteratorValueType<InnerIterT>;
  using difference_type =
      typename std::iterator_traits<InnerIterT>::difference_type;
  using pointer = typename std::iterator_traits<InnerIterT>::pointer;
  using reference = typename std::iterator_traits<InnerIterT>::reference;

  FlattenedIterator() : outer_it_{}, outer_end_{}, inner_it_{} {}

  FlattenedIterator(OuterIterT outer_it, OuterIterT outer_end,
                    InnerIterT inner_it = InnerIterT{})
      : outer_it_{outer_it}, outer_end_{outer_end}, inner_it_{inner_it} {}

  reference operator*() const { return *inner_it_; }

  pointer operator->() const { return &(*inner_it_); }

  FlattenedIterator &operator++() {
    ++inner_it_;
    if (inner_it_ == std::end(*outer_it_)) {
      ++outer_it_;
      if (outer_it_ != outer_end_) {
        inner_it_ = std::begin(*outer_it_);
      }
    }
    return *this;
  }

  FlattenedIterator operator++(int) {
    FlattenedIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  bool Equals(const FlattenedIterator &other) const {
    return outer_it_ == other.outer_it_ && inner_it_ == other.inner_it_;
  }

 private:
  OuterIterT outer_it_;
  OuterIterT outer_end_;
  InnerIterT inner_it_;
};

template <class OuterIterT>
bool operator==(const FlattenedIterator<OuterIterT> &lhs,
                const FlattenedIterator<OuterIterT> &rhs) {
  return lhs.Equals(rhs);
}

template <class OuterIterT>
bool operator!=(const FlattenedIterator<OuterIterT> &lhs,
                const FlattenedIterator<OuterIterT> &rhs) {
  return !lhs.Equals(rhs);
}

/**
 * @brief An unordered stable data structure which stores elements in a contiguous
 * array. It supports indexing and all operations (insertion, deletion,
 * indexing) takes O(1).
 *
 * @tparam T The type of theTInit() elements.
 * @tparam TInit is a callable object for default initalization of the elements.
 *
 * WARNING: this class doesn't handle object's lifetime, so dtors of stored
 * elements are called only when the DataPool is destructed.
 */
template <class T>
class DataPool final {
 public:
  using Iterator = FlattenedIterator<typename std::vector<Bucket<T>>::iterator>;
  using ConstIterator =
      FlattenedIterator<typename std::vector<Bucket<T>>::const_iterator>;

  explicit DataPool(std::function<T()> initializer = kDefaultCtor<T>)
      : initializer_{initializer} {}

  /**
   * @brief Inserts a new element into the DataPool.
   *
   * @param val The value to be inserted.
   * @return The index of the inserted element.
   */
  std::size_t Insert(const T &val) {
    if (partially_filled_buckets_.empty()) {
      std::size_t bucket_idx = buckets_.size();
      Bucket<T> &bucket = buckets_.emplace_back(initializer_());
      partially_filled_buckets_.push_back(bucket_idx);
      bucket.Insert(val);
      return bucket_idx * Bucket<T>::Capacity();
    }
    std::size_t bucket_idx = partially_filled_buckets_.back();
    Bucket<T> &bucket = buckets_[bucket_idx];
    std::size_t offset = bucket.Insert(val);
    if (bucket.Full() == 0) {
      partially_filled_buckets_.pop_back();
    }
    return bucket_idx * Bucket<T>::Capacity() + offset;
  }

  /**
   * @brief Inserts a new element into the DataPool.
   *
   * @param val The value to be inserted.
   * @return The index of the inserted element.
   */
  std::size_t Insert(T &&val) {
    if (partially_filled_buckets_.empty()) {
      std::size_t bucket_idx = buckets_.size();
      Bucket<T> &bucket = buckets_.emplace_back(initializer_());
      partially_filled_buckets_.push_back(bucket_idx);
      bucket.Insert(std::move(val));
      return bucket_idx * Bucket<T>::Capacity();
    }
    std::size_t bucket_idx = partially_filled_buckets_.back();
    Bucket<T> &bucket = buckets_[bucket_idx];
    std::size_t offset = bucket.Insert(std::move(val));
    if (bucket.Full() == 0) {
      partially_filled_buckets_.pop_back();
    }
    return bucket_idx * Bucket<T>::Capacity() + offset;
  }

  // If the element exists, erase it. Otherwise, leave the container as is.
  void Erase(std::size_t idx) {
    if (!Contains(idx)) {
      return;
    }
    std::size_t bucket_idx = idx / Bucket<T>::Capacity();
    Bucket<T> &bucket = buckets_[bucket_idx];
    if (bucket.Full() == 0) {
      partially_filled_buckets_.push_back(bucket_idx);
    }
    bucket.Erase(idx % Bucket<T>::Capacity());
  }

  bool Contains(std::size_t idx) const noexcept {
    std::size_t bucket_idx = idx / Bucket<T>::Capacity();
    if (bucket_idx >= buckets_.size()) {
      return false;
    }
    return buckets_[bucket_idx].Contains(idx % Bucket<T>::Capacity());
  }

  /**
   * @brief Returns a pointer to the element at the specified index.
   *
   * @param idx The index of the element.
   * @return A pointer to the element if it exists, otherwise nullptr.
   */
  T *At(std::size_t idx) noexcept {
    return Contains(idx) ? &UnsafeAt(idx) : nullptr;
  }

  /**
   * @brief Returns a pointer to the element at the specified index.
   *
   * @param idx The index of the element.
   * @return A pointer to the element if it exists, otherwise nullptr.
   */
  const T *At(std::size_t idx) const noexcept {
    return Contains(idx) ? &UnsafeAt(idx) : nullptr;
  }

  /**
   * @brief Returns a reference to the element at the specified index.
   *
   * @param idx The index of the element.
   * @return A reference to the element.
   *
   * This method returns a reference to the element at the specified index.
   * It assumes that the element exists and does not perform any bounds
   * checking.
   */
  T &operator[](std::size_t idx) noexcept { return UnsafeAt(idx); }

  /**
   * @brief Returns a reference to the element at the specified index.
   *
   * @param idx The index of the element.
   * @return A reference to the element.
   *
   * This method returns a reference to the element at the specified index.
   * It assumes that the element exists and does not perform any bounds
   * checking.
   */
  const T &operator[](std::size_t idx) const noexcept { return UnsafeAt(idx); }

  Iterator begin() noexcept {
    if (buckets_.empty()) {
      return FlattenedIterator{buckets_.begin(), buckets_.end()};
    }
    return FlattenedIterator{buckets_.begin(), buckets_.end(),
                             buckets_.front().begin()};
  }

  Iterator end() noexcept {
    if (buckets_.empty()) {
      return FlattenedIterator{buckets_.end(), buckets_.end()};
    }
    return FlattenedIterator{buckets_.end(), buckets_.end(),
                             buckets_.back().end()};
  }

  ConstIterator begin() const noexcept {
    if (buckets_.empty()) {
      return FlattenedIterator{buckets_.begin(), buckets_.end()};
    }
    return FlattenedIterator{buckets_.begin(), buckets_.end(),
                             buckets_.front().begin()};
  }

  ConstIterator end() const noexcept {
    if (buckets_.empty()) {
      return FlattenedIterator{buckets_.end(), buckets_.end()};
    }
    return FlattenedIterator{buckets_.end(), buckets_.end(),
                             buckets_.back().end()};
  }

 private:
  T &UnsafeAt(std::size_t idx) noexcept {
    std::size_t bucket_idx = idx / Bucket<T>::Capacity();
    std::size_t bucket_offset = idx % Bucket<T>::Capacity();
    Bucket<T> &bucket = buckets_[bucket_idx];
    assert(bucket.Contains(bucket_offset) && "Element doesn't exist");
    return bucket[bucket_offset];
  }

  const T &UnsafeAt(std::size_t idx) const noexcept {
    std::size_t bucket_idx = idx / Bucket<T>::Capacity();
    std::size_t bucket_offset = idx % Bucket<T>::Capacity();
    const Bucket<T> &bucket = buckets_[bucket_idx];
    assert(bucket.Contains(bucket_offset) && "Element doesn't exist");
    return bucket[bucket_offset];
  }

  std::vector<Bucket<T>> buckets_;
  std::vector<std::size_t> partially_filled_buckets_;
  std::function<T()> initializer_;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_DATA_POOL_H_
