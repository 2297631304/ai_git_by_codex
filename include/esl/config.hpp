#pragma once

#include <string>
#include <vector>

namespace esl {

struct FieldAudit {
    std::string name;
    std::string xml_text;
    std::string cpp_value;
    int line{0};
};

struct ModelConfig {
    std::string model_name;
    int clock_period_ns{0};
    int cycles{0};
    int threshold{0};
    int gain{0};
    bool enable_trace{false};
    std::vector<int> input_sequence;
    std::vector<int> mask_bits;
    std::string hazard_mode;
    int hazard_cycle{0};
    int hazard_read_offset{0};
};

ModelConfig load_config_xml(const std::string& path, std::vector<FieldAudit>& audit);

std::string vector_to_csv(const std::vector<int>& values);

} // namespace esl
