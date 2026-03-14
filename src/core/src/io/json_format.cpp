/**
 * @file json_format.cpp
 * @brief Implements JSON formatting helpers for IO text output.
 */
#include "skymapf/io/json_format.hpp"

#include <sstream>
#include <string>

namespace skymapf::io {

namespace {

using json = nlohmann::ordered_json;

void append_value(
    std::ostringstream& out,
    const json& value,
    int level,
    int indent_spaces
);

std::string indent(int level, int indent_spaces) {
    return std::string(static_cast<std::size_t>(level * indent_spaces), ' ');
}

void append_array_inline(std::ostringstream& out, const json& value) {
    out << "[";
    for (std::size_t i = 0; i < value.size(); ++i) {
        append_value(out, value[i], 0, 0);
        if (i + 1 < value.size()) {
            out << ", ";
        }
    }
    out << "]";
}

void append_object_pretty(
    std::ostringstream& out,
    const json& value,
    int level,
    int indent_spaces
) {
    if (value.empty()) {
        out << "{}";
        return;
    }

    out << "{\n";
    std::size_t i = 0;
    for (auto it = value.begin(); it != value.end(); ++it, ++i) {
        out << indent(level + 1, indent_spaces) << json(it.key()).dump() << ": ";
        append_value(out, it.value(), level + 1, indent_spaces);
        if (i + 1 < value.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << indent(level, indent_spaces) << "}";
}

void append_value(
    std::ostringstream& out,
    const json& value,
    int level,
    int indent_spaces
) {
    if (value.is_object()) {
        append_object_pretty(out, value, level, indent_spaces);
        return;
    }
    if (value.is_array()) {
        append_array_inline(out, value);
        return;
    }
    out << value.dump();
}

}  // namespace

std::string dump_json_inline_arrays(const nlohmann::ordered_json& value, int indent_spaces) {
    std::ostringstream out;
    append_value(out, value, 0, indent_spaces);
    out << "\n";
    return out.str();
}

}  // namespace skymapf::io
