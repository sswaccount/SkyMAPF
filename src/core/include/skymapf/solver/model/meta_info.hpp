/**
 * @file meta_info.hpp
 * @brief Defines metadata container for serialized solver-related models.
 */
#pragma once

#include <string>
#include <unordered_map>

namespace skymapf::common {

/**
 * @brief Generic metadata fields shared across model objects.
 */
struct MetaInfo {
    std::string schema_version;
    std::string name;
    std::string description;
    std::string created_at;
    std::string updated_at;
    std::unordered_map<std::string, std::string> tags;
};

}  // namespace skymapf::common