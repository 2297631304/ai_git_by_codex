#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace esl {

class DynamicBitset {
public:
    explicit DynamicBitset(std::size_t bit_count = 0)
        : bit_count_(bit_count), words_((bit_count + kWordBits - 1) / kWordBits, 0U) {}

    std::size_t size() const noexcept { return bit_count_; }

    void set(std::size_t index, bool value) {
        check_index(index);
        const auto word = index / kWordBits;
        const auto bit = index % kWordBits;
        const std::uint64_t mask = std::uint64_t{1} << bit;
        if (value) {
            words_.at(word) |= mask;
        } else {
            words_.at(word) &= ~mask;
        }
    }

    bool test(std::size_t index) const {
        check_index(index);
        const auto word = index / kWordBits;
        const auto bit = index % kWordBits;
        return (words_.at(word) & (std::uint64_t{1} << bit)) != 0U;
    }

    std::size_t count() const noexcept {
        std::size_t ones = 0;
        for (auto word : words_) {
            while (word != 0U) {
                word &= (word - 1U);
                ++ones;
            }
        }
        return ones;
    }

    std::string to_string() const {
        std::string text;
        text.reserve(bit_count_);
        for (std::size_t i = 0; i < bit_count_; ++i) {
            text.push_back(test(i) ? '1' : '0');
        }
        return text;
    }

private:
    static constexpr std::size_t kWordBits = 64;

    void check_index(std::size_t index) const {
        if (index >= bit_count_) {
            std::ostringstream oss;
            oss << "DynamicBitset index " << index << " is out of range, size=" << bit_count_;
            throw std::out_of_range(oss.str());
        }
    }

    std::size_t bit_count_{0};
    std::vector<std::uint64_t> words_;
};

} // namespace esl
