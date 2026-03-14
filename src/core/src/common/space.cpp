#include "skymapf/common/space.hpp"

namespace skymapf::common {

void to_json(nlohmann::ordered_json& j, const SpaceSpec& space_spec)
{
    j = nlohmann::ordered_json{
        {"kind", space_spec.is_2d() ? "2d" : "3d"},
        {"cols", space_spec.cols_},
        {"rows", space_spec.rows_},
        {"layers", space_spec.layers_}
    };
}

void from_json(const nlohmann::json& j, SpaceSpec& space_spec)
{
    const std::string kind = j.at("kind").get<std::string>();
    const std::int32_t cols = j.at("cols").get<std::int32_t>();
    const std::int32_t rows = j.at("rows").get<std::int32_t>();

    if (kind == "2d") {
        const std::int32_t layers = j.value("layers", 1);
        if (layers != 1) {
            throw std::runtime_error("SpaceSpec 2d requires layers == 1");
        }
        space_spec = SpaceSpec::make_2d(cols, rows);
    } else if (kind == "3d") {
        const std::int32_t layers = j.at("layers").get<std::int32_t>();
        space_spec = SpaceSpec::make_3d(cols, rows, layers);
    } else {
        throw std::runtime_error("SpaceSpec kind must be '2d' or '3d'");
    }

    if (!space_spec.is_valid()) {
        throw std::runtime_error("SpaceSpec contains invalid dimensions");
    }
}

}