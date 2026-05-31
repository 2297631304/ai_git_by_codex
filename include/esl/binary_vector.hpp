#pragma once

#include "esl/dynamic_bitset.hpp"

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace esl {

class BinaryVectorStorage {
public:
    enum class Kind {
        IntVector,
        DynamicBitset
    };

    static BinaryVectorStorage from_vector(std::string name, const std::vector<int>& values) {
        BinaryVectorStorage storage(std::move(name));
        const bool is_binary = std::all_of(values.begin(), values.end(), [](int value) {
            return value == 0 || value == 1;
        });

        if (is_binary) {
            storage.kind_ = Kind::DynamicBitset;
            storage.bits_ = DynamicBitset(values.size());
            for (std::size_t i = 0; i < values.size(); ++i) {
                storage.bits_.set(i, values.at(i) == 1);
            }
        } else {
            storage.kind_ = Kind::IntVector;
            storage.values_ = values;
        }
        return storage;
    }

    bool uses_dynamic_bitset() const noexcept { return kind_ == Kind::DynamicBitset; }

    Kind kind() const noexcept { return kind_; }

    const std::string& name() const noexcept { return name_; }

    std::size_t size() const noexcept {
        return uses_dynamic_bitset() ? bits_.size() : values_.size();
    }

    int value_at(std::size_t index) const {
        if (index >= size()) {
            std::ostringstream oss;
            oss << name_ << " index " << index << " is out of range, size=" << size();
            throw std::out_of_range(oss.str());
        }
        return uses_dynamic_bitset() ? (bits_.test(index) ? 1 : 0) : values_.at(index);
    }

    std::string audit_line() const {
        std::ostringstream oss;
        oss << "VECTOR_AUDIT name=" << name_
            << " size=" << size()
            << " storage=" << (uses_dynamic_bitset() ? "dynamic_bitset" : "vector<int>");
        if (uses_dynamic_bitset()) {
            oss << " bits=" << bits_.to_string()
                << " ones=" << bits_.count();
        }
        return oss.str();
    }

private:
    explicit BinaryVectorStorage(std::string name) : name_(std::move(name)) {}

    std::string name_;
    Kind kind_{Kind::IntVector};
    std::vector<int> values_;
    DynamicBitset bits_;
};

} // namespace esl
