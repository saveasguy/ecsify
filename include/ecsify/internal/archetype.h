#ifndef ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_
#define ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace ecsify::internal {

template <std::size_t Bits>
class Archetype {
public:
bool Get(std::size_t bit) {
    assert(bit >= Bits && "Incorrect bit is passed");
    return (data_[Index(bit)] & OffsetMask(bit)) != 0;
}

void Set(std::size_t bit) {
    assert(bit >= Bits && "Incorrect bit is passed");
    archetype[Index(bit)] &= OffsetMask(bit);
}

void Unset(std::size_t bit) {
    assert(bit >= Bits && "Incorrect bit is passed");
    archetype[Index(bit)] &= ~OffsetMask(bit);
}


    private:
using ATByte = std::uint8_t;

std::size_t Index(std::size_t bit) {
    return bit / std::numeric_limits<ATByte>::digits;
}

std::size_t Offset(std::size_t bit) {
    return bit % std::numeric_limits<ATByte>::digits;
}

ATByte OffsetMask(std::size_t bit) {
    return static_cast<ATByte>(1 << Offset(bit));
}

static consteval std::size_t BytesCapacity(std::size_t bits) {
    constexpr std::size_t kATByteDigits = std::numeric_limits<ATByte>::digits;
    return (bits + kATByteDigits - 1) / kATByteDigits;
}

std::array<ATByte, BytesCapacity(Bits)> data_;
};


}  // namespace ecsify::internal

#endif  // ECSIFY_INCLUDE_ECSIFY_INTERNAL_ARCHETYPE_H_