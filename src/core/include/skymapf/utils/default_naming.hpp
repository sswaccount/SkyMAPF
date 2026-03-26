/**
* @file default_naming.hpp
* @brief Defines default naming helpers for domain objects.
*/
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "skymapf/common/space.hpp"

namespace skymapf::utils {

/**
* @brief Configures suffix widths for different object categories.
*/
struct NamingConfig {
    std::size_t agent_suffix_width{6};
    std::size_t world_suffix_width{6};
    std::size_t task_suffix_width{6};
    std::size_t instance_suffix_width{6};
    std::size_t scenario_suffix_width{6};
};

/**
* @brief Provides default object naming helpers.
*
* Names are built from:
* - a semantic prefix
* - optional structured fields
* - one random base36 suffix derived from the global name RNG
*
* This helper relies on RandomTool::instance() for suffix generation.
*/
class DefaultNaming {
public:
    /**
    * @brief Replaces current naming configuration.
    */
    static void configure(const NamingConfig& config) noexcept;

    /**
    * @brief Returns current naming configuration.
    */
    static const NamingConfig& config() noexcept;

    /**
    * @brief Returns one default agent name.
    */
    static std::string next_agent_name();

    /**
    * @brief Returns one default world name.
    */
    static std::string next_world_name(const common::SpaceSpec& spec);

    /**
    * @brief Returns one default task name.
    */
    static std::string next_task_name(std::size_t agent_size);

    /**
    * @brief Returns one default instance name.
    */
    static std::string next_instance_name();

    /**
    * @brief Returns one default scenario name.
    */
    static std::string next_scenario_name();

private:
    static std::string next_suffix(std::size_t width);
    static std::string world_shape_tag(const common::SpaceSpec& spec);
    static std::string to_base36(std::uint64_t value, std::size_t width);
};

}  // namespace skymapf::common