#include "esl/checked_access.hpp"
#include "esl/filter_core.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace esl {

ThresholdAccumulatorCore::ThresholdAccumulatorCore(const ModelConfig& config)
    : input_sequence_(config.input_sequence),
      mask_bits_(BinaryVectorStorage::from_vector("mask_bits", config.mask_bits)),
      threshold_(config.threshold),
      gain_(config.gain),
      hazard_mode_(config.hazard_mode),
      hazard_cycle_(config.hazard_cycle),
      hazard_read_offset_(config.hazard_read_offset) {
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

    if (cycle == static_cast<std::uint64_t>(hazard_cycle_)) {
        const auto hazard_index = index + static_cast<std::size_t>(hazard_read_offset_);
        if (hazard_mode_ == "checked_oob") {
            (void)checked_at(input_sequence_, hazard_index, "input_sequence/hazard_checked");
        } else if (hazard_mode_ == "unchecked_oob") {
            // Deliberately unsafe demo path. Static checks must flag this direct operator[] access.
            (void)input_sequence_[hazard_index];
        } else if (hazard_mode_ == "hang") {
            std::cout << "HANG_DEMO cycle=" << cycle << " mode=hang\n";
            for (;;) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }

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
