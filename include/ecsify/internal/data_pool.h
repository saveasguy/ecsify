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
#include <limits>
#include <utility>
#include <vector>

namespace ecsify::internal {

template <typename IteratorT, typename MaskT>
class MaskedIterator {
 public:
  using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  using difference_type = typename std::iterator_traits<Iterator>::difference_type;
  using pointer = typename std::iterator_traits<Iterator>::pointer;
  using reference = typename std::iterator_traits<Iterator>::reference;

  using Iterator = IteratorT;
  using Mask = MaskT;

  MaskedIterator(Iterator it, Mask mask)
      : it_(it), mask_(mask) {
    advanceToValid();
  }

  reference operator*() const { return *it_; }
  pointer operator->() const { return &(*it_); }

  MaskedIterator& operator++() {
    ++it_;
    ++index_;
    advanceToValid();
    return *this;
  }

  MaskedIterator operator++(int) {
    MaskedIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const MaskedIterator& a, const MaskedIterator& b) {
    return a.it_ == b.it_;
  }

  friend bool operator!=(const MaskedIterator& a, const MaskedIterator& b) {
    return !(a == b);
  }

 private:
  void advanceToValid() {
    while (it_ != Iterator() && !isBitSet(index_)) {
      ++it_;
      ++index_;
    }
  }

  bool isBitSet(std::size_t idx) const {
    return (mask_ & (static_cast<std::uint64_t>(1) << idx)) != 0;
  }

  Iterator it_;
  Mask mask_;
};

template <class T>
  requires(std::default_initializable<T>)
const auto kDefaultCtor = []() { return T(); };

template <class T>
  requires(std::default_initializable<T> && std::copyable<T>)
class Bucket final {
 public:
  using Mask = std::uint64_t;

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

  static consteval std::size_t Capacity() noexcept {
    return std::numeric_limits<Mask>::digits;
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

/**
 * @brief An unordered data structure which stores elements in a contiguous
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
    return Contains(idx) ? &UnsafeAt_(idx) : nullptr;
  }

  /**
   * @brief Returns a pointer to the element at the specified index.
   *
   * @param idx The index of the element.
   * @return A pointer to the element if it exists, otherwise nullptr.
   */
  const T *At(std::size_t idx) const noexcept {
    return Contains(idx) ? &UnsafeAt_(idx) : nullptr;
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
  T &operator[](std::size_t idx) noexcept { return UnsafeAt_(idx); }

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
  const T &operator[](std::size_t idx) const noexcept { return UnsafeAt_(idx); }

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
