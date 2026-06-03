#pragma once

#include "esl/binary_vector.hpp"
#include "esl/config.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace esl {

struct TickResult {
    std::uint64_t cycle{0};
    std::size_t index{0};
    int sample{0};
    int mask{0};
    int contribution{0};
    int accumulator{0};
    bool fired{false};
};

class ThresholdAccumulatorCore {
public:
    explicit ThresholdAccumulatorCore(const ModelConfig& config);

    TickResult tick(std::uint64_t cycle);

    std::string storage_audit() const;

private:
    std::vector<int> input_sequence_;
    BinaryVectorStorage mask_bits_;
    int threshold_{0};
    int gain_{0};
    int accumulator_{0};
    std::string hazard_mode_;
    int hazard_cycle_{0};
    int hazard_read_offset_{0};
};

} // namespace esl
