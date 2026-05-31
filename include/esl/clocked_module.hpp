#pragma once

#include "esl/filter_core.hpp"
#include "esl/mini_systemc.hpp"

#include <iostream>
#include <utility>

namespace esl {

class ClockedFilterModule final : public sc_core::sc_module, public sc_core::clocked_process {
public:
    ClockedFilterModule(std::string name, const ModelConfig& config)
        : sc_core::sc_module(std::move(name)), core_(config), trace_enabled_(config.enable_trace) {}

    const ThresholdAccumulatorCore& core() const noexcept { return core_; }

    void posedge(std::uint64_t cycle, std::uint64_t time_ns) override {
        const auto result = core_.tick(cycle);
        if (trace_enabled_) {
            std::cout << "TRACE time=" << time_ns << "ns"
                      << " cycle=" << result.cycle
                      << " index=" << result.index
                      << " sample=" << result.sample
                      << " mask=" << result.mask
                      << " contribution=" << result.contribution
                      << " accumulator=" << result.accumulator
                      << " fired=" << (result.fired ? "true" : "false")
                      << '\n';
        }
    }

private:
    ThresholdAccumulatorCore core_;
    bool trace_enabled_{false};
};

} // namespace esl
