#include "esl/config.hpp"
#include "esl/config_schema.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace esl {
namespace {

struct XmlValue {
    std::string text;
    int line{0};
};

std::string trim(std::string text) {
    const auto not_space = [](unsigned char ch) { return ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n'; };
    text.erase(text.begin(), std::find_if(text.begin(), text.end(), not_space));
    text.erase(std::find_if(text.rbegin(), text.rend(), not_space).base(), text.end());
    return text;
}

int parse_int(const XmlValue& value, const std::string& name, int min_value, int max_value) {
    std::size_t pos = 0;
    int parsed = 0;
    try {
        parsed = std::stoi(trim(value.text), &pos, 10);
    } catch (const std::exception&) {
        throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " is not an integer");
    }
    if (pos != trim(value.text).size()) {
        throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " has trailing text");
    }
    if (parsed < min_value || parsed > max_value) {
        std::ostringstream oss;
        oss << "XML field '" << name << "' at line " << value.line
            << " is outside range [" << min_value << ", " << max_value << ']';
        throw std::runtime_error(oss.str());
    }
    return parsed;
}

bool parse_bool(const XmlValue& value, const std::string& name) {
    const auto text = trim(value.text);
    if (text == "true" || text == "1") {
        return true;
    }
    if (text == "false" || text == "0") {
        return false;
    }
    throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " is not a bool");
}

std::vector<int> parse_int_vector(const XmlValue& value, const std::string& name) {
    std::vector<int> values;
    std::stringstream ss(value.text);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        if (token.empty()) {
            throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " contains an empty item");
        }
        std::size_t pos = 0;
        int parsed = 0;
        try {
            parsed = std::stoi(token, &pos, 10);
        } catch (const std::exception&) {
            throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " contains a non-integer item");
        }
        if (pos != token.size()) {
            throw std::runtime_error("XML field '" + name + "' at line " + std::to_string(value.line) + " contains trailing item text");
        }
        values.push_back(parsed);
    }
    if (values.empty()) {
        throw std::runtime_error("XML field '" + name + "' must not be empty");
    }
    return values;
}

const XmlValue& required(const std::map<std::string, XmlValue>& fields, const std::string& name) {
    const auto iter = fields.find(name);
    if (iter == fields.end()) {
        throw std::runtime_error("XML is missing required field '" + name + "'");
    }
    return iter->second;
}

void add_audit(std::vector<FieldAudit>& audit, const std::string& name, const XmlValue& xml, const std::string& cpp_value) {
    audit.push_back(FieldAudit{name, trim(xml.text), cpp_value, xml.line});
}

} // namespace

std::string vector_to_csv(const std::vector<int>& values) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0U) {
            oss << ',';
        }
        oss << values.at(i);
    }
    return oss.str();
}

ModelConfig load_config_xml(const std::string& path, std::vector<FieldAudit>& audit) {
    std::set<std::string> allowed;
    for (const auto& field : config_schema()) {
        allowed.insert(field.name);
    }

    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open XML config: " + path);
    }

    std::map<std::string, XmlValue> fields;
    bool inside_root = false;
    bool closed_root = false;
    std::string line;
    int line_number = 0;
    const std::regex element_re(R"(^\s*<([A-Za-z_][A-Za-z0-9_]*)>(.*)</([A-Za-z_][A-Za-z0-9_]*)>\s*$)");

    while (std::getline(input, line)) {
        ++line_number;
        const auto text = trim(line);
        if (text.empty() || text.rfind("<?xml", 0) == 0 || text.rfind("<!--", 0) == 0) {
            continue;
        }
        if (text == "<esl_model>") {
            if (inside_root) {
                throw std::runtime_error("Duplicate XML root at line " + std::to_string(line_number));
            }
            inside_root = true;
            continue;
        }
        if (text == "</esl_model>") {
            closed_root = true;
            inside_root = false;
            continue;
        }
        if (!inside_root || closed_root) {
            throw std::runtime_error("Unexpected XML content at line " + std::to_string(line_number) + ": " + text);
        }

        std::smatch match;
        if (!std::regex_match(line, match, element_re)) {
            throw std::runtime_error("Only single-line scalar XML fields are supported, parse failed at line " + std::to_string(line_number));
        }
        const std::string open_tag = match[1].str();
        const std::string close_tag = match[3].str();
        if (open_tag != close_tag) {
            throw std::runtime_error("XML tag mismatch at line " + std::to_string(line_number) + ": " + open_tag + " vs " + close_tag);
        }
        if (allowed.find(open_tag) == allowed.end()) {
            throw std::runtime_error("Unknown XML field '" + open_tag + "' at line " + std::to_string(line_number));
        }
        if (fields.find(open_tag) != fields.end()) {
            throw std::runtime_error("Duplicate XML field '" + open_tag + "' at line " + std::to_string(line_number));
        }
        fields.emplace(open_tag, XmlValue{match[2].str(), line_number});
    }

    if (!closed_root) {
        throw std::runtime_error("XML root <esl_model> was not closed");
    }

    ModelConfig config;
    config.model_name = trim(required(fields, "model_name").text);
    config.clock_period_ns = parse_int(required(fields, "clock_period_ns"), "clock_period_ns", 1, 1000000);
    config.cycles = parse_int(required(fields, "cycles"), "cycles", 1, 1000000);
    config.threshold = parse_int(required(fields, "threshold"), "threshold", 1, 1000000);
    config.gain = parse_int(required(fields, "gain"), "gain", 1, 1000000);
    config.enable_trace = parse_bool(required(fields, "enable_trace"), "enable_trace");
    config.input_sequence = parse_int_vector(required(fields, "input_sequence"), "input_sequence");
    config.mask_bits = parse_int_vector(required(fields, "mask_bits"), "mask_bits");
    config.hazard_mode = trim(required(fields, "hazard_mode").text);
    config.hazard_cycle = parse_int(required(fields, "hazard_cycle"), "hazard_cycle", 0, 1000000);
    config.hazard_read_offset = parse_int(required(fields, "hazard_read_offset"), "hazard_read_offset", 0, 1000000);

    if (config.model_name.empty()) {
        throw std::runtime_error("XML field 'model_name' must not be empty");
    }
    if (config.input_sequence.size() != config.mask_bits.size()) {
        throw std::runtime_error("XML fields 'input_sequence' and 'mask_bits' must have the same item count");
    }
    if (!std::all_of(config.mask_bits.begin(), config.mask_bits.end(), [](int value) { return value == 0 || value == 1; })) {
        throw std::runtime_error("XML field 'mask_bits' must contain only 0/1 values");
    }
    const std::set<std::string> hazard_modes{"off", "checked_oob", "unchecked_oob", "hang"};
    if (hazard_modes.find(config.hazard_mode) == hazard_modes.end()) {
        throw std::runtime_error("XML field 'hazard_mode' must be one of off, checked_oob, unchecked_oob, hang");
    }

    add_audit(audit, "model_name", required(fields, "model_name"), config.model_name);
    add_audit(audit, "clock_period_ns", required(fields, "clock_period_ns"), std::to_string(config.clock_period_ns));
    add_audit(audit, "cycles", required(fields, "cycles"), std::to_string(config.cycles));
    add_audit(audit, "threshold", required(fields, "threshold"), std::to_string(config.threshold));
    add_audit(audit, "gain", required(fields, "gain"), std::to_string(config.gain));
    add_audit(audit, "enable_trace", required(fields, "enable_trace"), config.enable_trace ? "true" : "false");
    add_audit(audit, "input_sequence", required(fields, "input_sequence"), vector_to_csv(config.input_sequence));
    add_audit(audit, "mask_bits", required(fields, "mask_bits"), vector_to_csv(config.mask_bits));
    add_audit(audit, "hazard_mode", required(fields, "hazard_mode"), config.hazard_mode);
    add_audit(audit, "hazard_cycle", required(fields, "hazard_cycle"), std::to_string(config.hazard_cycle));
    add_audit(audit, "hazard_read_offset", required(fields, "hazard_read_offset"), std::to_string(config.hazard_read_offset));

    return config;
}

} // namespace esl
