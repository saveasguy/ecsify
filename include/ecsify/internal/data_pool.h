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

template <class T>
  requires(std::default_initializable<T>)
const auto kDefaultCtor = []() { return T(); };

template <class T>
concept DataPoolCompatible = std::default_initializable<T> && std::copyable<T>;

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
  requires(DataPoolCompatible<T>)
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
      Bucket &bucket = buckets_.emplace_back(initializer_());
      partially_filled_buckets_.push_back(bucket_idx);
      bucket.Insert(val);
      return bucket_idx * Bucket::Capacity();
    }
    std::size_t bucket_idx = partially_filled_buckets_.back();
    Bucket &bucket = buckets_[bucket_idx];
    std::size_t offset = bucket.Insert(val);
    if (bucket.Full() == 0) {
      partially_filled_buckets_.pop_back();
    }
    return bucket_idx * Bucket::Capacity() + offset;
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
      Bucket &bucket = buckets_.emplace_back(initializer_());
      partially_filled_buckets_.push_back(bucket_idx);
      bucket.Insert(std::move(val));
      return bucket_idx * Bucket::Capacity();
    }
    std::size_t bucket_idx = partially_filled_buckets_.back();
    Bucket &bucket = buckets_[bucket_idx];
    std::size_t offset = bucket.Insert(std::move(val));
    if (bucket.Full() == 0) {
      partially_filled_buckets_.pop_back();
    }
    return bucket_idx * Bucket::Capacity() + offset;
  }

  // If the element exists, erase it. Otherwise, leave the container as is.
  void Erase(std::size_t idx) {
    if (!Contains(idx)) {
      return;
    }
    std::size_t bucket_idx = idx / Bucket::Capacity();
    Bucket &bucket = buckets_[bucket_idx];
    if (bucket.Full() == 0) {
      partially_filled_buckets_.push_back(bucket_idx);
    }
    bucket.Erase(idx % Bucket::Capacity());
  }

  bool Contains(std::size_t idx) const noexcept {
    std::size_t bucket_idx = idx / Bucket::Capacity();
    if (bucket_idx >= buckets_.size()) {
      return false;
    }
    return buckets_[bucket_idx].Contains(idx % Bucket::Capacity());
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
  class Bucket final {
   public:
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
      assert(idx < kCapacity && "Index out of bounds");
      return (free_elements_mask_ & GetMaskWithNthBitSet(idx)) == 0;
    }

    bool Full() const noexcept { return free_elements_mask_ == 0; }

    static consteval std::size_t Capacity() noexcept { return kCapacity; }

   private:
    using Mask = std::uint64_t;
    static constexpr std::size_t kCapacity = std::numeric_limits<Mask>::digits;

    static Mask GetMaskWithNthBitSet(std::size_t n) noexcept {
      assert(n < kCapacity && "Shift number is too large");
      return static_cast<Mask>(1) << n;
    }

    std::array<T, kCapacity> data_{};
    // 1 means free, 0 means occupied.
    Mask free_elements_mask_ = std::numeric_limits<Mask>::max();
  };

  T &UnsafeAt_(std::size_t idx) noexcept {
    std::size_t bucket_idx = idx / Bucket::Capacity();
    std::size_t bucket_offset = idx % Bucket::Capacity();
    Bucket &bucket = buckets_[bucket_idx];
    assert(bucket.Contains(bucket_offset) && "Element doesn't exist");
    return bucket[bucket_offset];
  }

  const T &UnsafeAt_(std::size_t idx) const noexcept {
    std::size_t bucket_idx = idx / Bucket::Capacity();
    std::size_t bucket_offset = idx % Bucket::Capacity();
    const Bucket &bucket = buckets_[bucket_idx];
    assert(bucket.Contains(bucket_offset) && "Element doesn't exist");
    return bucket[bucket_offset];
  }

  std::vector<Bucket> buckets_;
  std::vector<std::size_t> partially_filled_buckets_;
  std::function<T()> initializer_;
};

}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_DATA_POOL_H_
