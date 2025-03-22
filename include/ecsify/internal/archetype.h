#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <ranges>

namespace ecsify::internal {

template <std::size_t Bits>
class Archetype;

template <std::size_t Bits>
class ArchetypeIterator final {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = bool;

  ArchetypeIterator() : archetype_{nullptr}, idx_{0} {}

  ArchetypeIterator(const Archetype<Bits> *archetype, std::size_t idx)
      : archetype_{archetype}, idx_{idx} {}

  value_type operator*() const { return archetype_->At(idx_); }

  ArchetypeIterator &operator++() {
    ++idx_;
    return *this;
  }

  ArchetypeIterator operator++(int) {
    ArchetypeIterator copy = *this;
    ++idx_;
    return copy;
  }

  template <std::size_t N>
  friend bool operator==(const ArchetypeIterator<N> &lhs,
                         const ArchetypeIterator<N> &rhs);

 private:
  const Archetype<Bits> *archetype_;
  std::size_t idx_;
};

template <std::size_t Bits>
bool operator==(const ArchetypeIterator<Bits> &lhs,
                const ArchetypeIterator<Bits> &rhs) {
  return lhs.archetype_ == rhs.archetype_ && lhs.idx_ == rhs.idx_;
}

template <std::size_t Bits>
class ConstArchetypeIterator final {
 public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = bool;

  ConstArchetypeIterator() : archetype_{nullptr}, idx_{0} {}

  ConstArchetypeIterator(const Archetype<Bits> *archetype, std::size_t idx)
      : archetype_{archetype}, idx_{idx} {}

  value_type operator*() const { return archetype_->At(idx_); }

  ConstArchetypeIterator &operator++() {
    ++idx_;
    return *this;
  }

  ConstArchetypeIterator operator++(int) {
    ConstArchetypeIterator copy = *this;
    ++idx_;
    return copy;
  }

  template <std::size_t N>
  friend bool operator==(const ConstArchetypeIterator<N> &lhs,
                         const ConstArchetypeIterator<N> &rhs);

 private:
  const Archetype<Bits> *archetype_;
  std::size_t idx_;
};

template <std::size_t Bits>
bool operator==(const ConstArchetypeIterator<Bits> &lhs,
                const ConstArchetypeIterator<Bits> &rhs) {
  return lhs.archetype_ == rhs.archetype_ && lhs.idx_ == rhs.idx_;
}

template <std::size_t Bits>
class Archetype {
 public:
  bool At(std::size_t bit) const noexcept {
    assert(bit < Bits && "Incorrect bit is passed");
    return (data_[Index(bit)] & OffsetMask(bit)) != 0;
  }

  void Set(std::size_t bit) noexcept {
    assert(bit < Bits && "Incorrect bit is passed");
    data_[Index(bit)] |= OffsetMask(bit);
  }

  void Unset(std::size_t bit) noexcept {
    assert(bit < Bits && "Incorrect bit is passed");
    data_[Index(bit)] &= ~OffsetMask(bit);
  }

  std::size_t Hash() const noexcept {
    std::size_t result = 0;
    for (std::uint64_t val : data_) {
      result ^= val;
    }
    return result;
  }

  bool IsPrefix(const Archetype<Bits> &other) const noexcept {
    for (auto [val, other_val] : std::views::zip(data_, other.data_)) {
      if (val & other_val != val) {
        return false;
      }
    }
    return true;
  }

  std::size_t Size() const noexcept { return Bits; }

  ArchetypeIterator<Bits> begin() { return ArchetypeIterator(this, 0); }
  ArchetypeIterator<Bits> end() { return ArchetypeIterator(this, Bits); }

  ConstArchetypeIterator<Bits> begin() const {
    return ConstArchetypeIterator<Bits>(this, 0);
  }
  ConstArchetypeIterator<Bits> end() const {
    return ConstArchetypeIterator<Bits>(this, Bits);
  }

  template <std::size_t N>
  friend bool operator==(const Archetype<N> &lhs, const Archetype<N> &rhs);

 private:
  static constexpr std::size_t Index(std::size_t bit) {
    return bit / std::numeric_limits<std::uint64_t>::digits;
  }

  static constexpr std::uint64_t OffsetMask(std::size_t bit) {
    std::size_t offset = bit % std::numeric_limits<std::uint64_t>::digits;
    return static_cast<std::uint64_t>(1 << offset);
  }

  static consteval std::size_t UnderlyingCapacity(std::size_t bits) {
    constexpr std::size_t kDigits = std::numeric_limits<std::uint64_t>::digits;
    return (bits + kDigits - 1) / kDigits;
  }

  std::array<std::uint64_t, UnderlyingCapacity(Bits)> data_{};
};

template <std::size_t N>
bool operator==(const Archetype<N> &lhs, const Archetype<N> &rhs) {
  for (auto [lhs_val, rhs_val] : std::views::zip(lhs.data_, rhs.data_)) {
    if (lhs_val != rhs_val) {
      return false;
    }
  }
  return true;
}

}  // namespace ecsify::internal

template <std::size_t N>
struct std::hash<ecsify::internal::Archetype<N>> {
  std::size_t operator()(
      const ecsify::internal::Archetype<N> &archetype) const noexcept {
    return archetype.Hash();
  }
};

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_
