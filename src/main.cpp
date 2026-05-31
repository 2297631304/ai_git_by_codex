#include "esl/clocked_module.hpp"
#include "esl/config.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    const std::string config_path = argc > 1 ? argv[1] : "config/esl_config.xml";

    try {
        std::vector<esl::FieldAudit> audit;
        const auto config = esl::load_config_xml(config_path, audit);

        std::cout << "ESL_MODEL name=" << config.model_name << '\n';
        for (const auto& field : audit) {
            std::cout << "CONFIG_AUDIT field=" << field.name
                      << " line=" << field.line
                      << " xml=\"" << field.xml_text << "\""
                      << " cpp=\"" << field.cpp_value << "\""
                      << " status=loaded"
                      << '\n';
        }

        sc_core::sc_clock clock("clk", static_cast<std::uint64_t>(config.clock_period_ns));
        esl::ClockedFilterModule module("u_threshold_accumulator", config);
        sc_core::simple_kernel kernel;
        kernel.add(module);

        std::cout << module.core().storage_audit() << '\n';
        std::cout << "CLOCK name=" << clock.name()
                  << " period_ns=" << clock.period_ns()
                  << " cycles=" << config.cycles
                  << '\n';

        kernel.start(clock, config.cycles);
        std::cout << "SIM_DONE time=" << sc_core::sc_time_stamp() << '\n';
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "ERROR " << ex.what() << '\n';
        return 1;
    }
}
