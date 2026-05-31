#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace sc_core {

class sc_clock {
public:
    sc_clock(std::string name, std::uint64_t period_ns)
        : name_(std::move(name)), period_ns_(period_ns) {}

    const std::string& name() const noexcept { return name_; }
    std::uint64_t period_ns() const noexcept { return period_ns_; }

private:
    std::string name_;
    std::uint64_t period_ns_{0};
};

class sc_module {
public:
    explicit sc_module(std::string name) : name_(std::move(name)) {}
    virtual ~sc_module() = default;

    const std::string& name() const noexcept { return name_; }

private:
    std::string name_;
};

class clocked_process {
public:
    virtual ~clocked_process() = default;
    virtual void posedge(std::uint64_t cycle, std::uint64_t time_ns) = 0;
};

class simple_kernel {
public:
    void add(clocked_process& process) {
        processes_.push_back(&process);
    }

    void start(const sc_clock& clock, int cycles) {
        for (int cycle = 0; cycle < cycles; ++cycle) {
            const auto now = static_cast<std::uint64_t>(cycle) * clock.period_ns();
            time_ns_ = now;
            for (auto* process : processes_) {
                process->posedge(static_cast<std::uint64_t>(cycle), now);
            }
        }
    }

    static std::uint64_t time_stamp_ns() noexcept { return time_ns_; }

private:
    inline static std::uint64_t time_ns_{0};
    std::vector<clocked_process*> processes_;
};

inline std::string sc_time_stamp() {
    std::ostringstream oss;
    oss << simple_kernel::time_stamp_ns() << " ns";
    return oss.str();
}

} // namespace sc_core
