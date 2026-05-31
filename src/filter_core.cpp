#include "esl/filter_core.hpp"

#include <sstream>
#include <stdexcept>

namespace esl {

ThresholdAccumulatorCore::ThresholdAccumulatorCore(const ModelConfig& config)
    : input_sequence_(config.input_sequence),
      mask_bits_(BinaryVectorStorage::from_vector("mask_bits", config.mask_bits)),
      threshold_(config.threshold),
      gain_(config.gain) {
    if (input_sequence_.empty()) {
        throw std::runtime_error("input_sequence must not be empty");
    }
    if (input_sequence_.size() != mask_bits_.size()) {
        throw std::runtime_error("input_sequence and mask_bits size mismatch");
    }
}

TickResult ThresholdAccumulatorCore::tick(std::uint64_t cycle) {
    const auto index = static_cast<std::size_t>(cycle % input_sequence_.size());
    const int sample = input_sequence_.at(index);
    const int mask = mask_bits_.value_at(index);
    const int contribution = (mask == 1) ? sample * gain_ : 0;
    accumulator_ += contribution;

    bool fired = false;
    if (accumulator_ >= threshold_) {
        fired = true;
        accumulator_ -= threshold_;
    }

    return TickResult{
        cycle,
        index,
        sample,
        mask,
        contribution,
        accumulator_,
        fired,
    };
}

std::string ThresholdAccumulatorCore::storage_audit() const {
    return mask_bits_.audit_line();
}

} // namespace esl
