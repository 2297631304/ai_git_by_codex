#pragma once

#include <string>
#include <vector>

namespace esl {

#define ESL_CONFIG_FIELD_LIST(X) \
    X(model_name, string) \
    X(clock_period_ns, int) \
    X(cycles, int) \
    X(threshold, int) \
    X(gain, int) \
    X(enable_trace, bool) \
    X(input_sequence, int_vector) \
    X(mask_bits, int_vector) \
    X(hazard_mode, string) \
    X(hazard_cycle, int) \
    X(hazard_read_offset, int)

struct ConfigFieldSpec {
    const char* name;
    const char* type;
};

inline std::vector<ConfigFieldSpec> config_schema() {
    return {
#define ESL_CONFIG_SCHEMA_ITEM(name, type) ConfigFieldSpec{#name, #type},
        ESL_CONFIG_FIELD_LIST(ESL_CONFIG_SCHEMA_ITEM)
#undef ESL_CONFIG_SCHEMA_ITEM
    };
}

inline std::vector<std::string> config_field_names() {
    std::vector<std::string> names;
    for (const auto& field : config_schema()) {
        names.emplace_back(field.name);
    }
    return names;
}

} // namespace esl
