/**
 * @file json_format.hpp
 * @brief Declares JSON formatting helpers for IO text output.
 */
#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace skymapf::io {

/**
 * @brief Dumps JSON with object indentation and inline array formatting.
 *
 * Formatting policy:
 * - Object fields use multi-line pretty output.
 * - Arrays are always emitted on a single line.
 *
 * @param value JSON value to serialize.
 * @param indent_spaces Spaces per indentation level for objects.
 * @return Formatted JSON string ending with '\n'.
 */
std::string dump_json_inline_arrays(
    const nlohmann::ordered_json& value,
    int indent_spaces = 2
);

}  // namespace skymapf::io
