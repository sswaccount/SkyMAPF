#pragma once

namespace skymapf::common {

struct MetaInfo {
    SchemaVersion schema_version;
    std::string name;
    std::string description;
    std::string created_at;
    std::string updated_at;
    std::unordered_map<std::string, std::string> tags;
};

};