/**
 * @file name_generator.hpp
 * @brief Defines a reusable sequential name generation utility.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace skymapf::utils {

/**
 * @brief Generates sequential names using a configurable prefix.
 *
 * Example output with default config: `item_1`, `item_2`, `item_3`.
 */
class NameGenerator {
public:
    /**
     * @brief Constructs a name generator.
     *
     * @param prefix Prefix added before each numeric suffix.
     * @param start_index First numeric suffix to emit.
     */
    explicit NameGenerator(std::string prefix = "item", std::uint64_t start_index = 1)
        : prefix_(std::move(prefix)), next_index_(start_index) {}

    /**
     * @brief Generates one name and advances the internal index.
     *
     * @return Generated name.
     */
    std::string next() {
        const auto index = next_index_++;
        return compose(index);
    }

    /**
     * @brief Generates multiple names and advances the internal index.
     *
     * @param count Number of names to generate.
     * @return Generated names in order.
     */
    std::vector<std::string> next_batch(std::size_t count) {
        std::vector<std::string> names;
        names.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            names.push_back(next());
        }
        return names;
    }

    /**
     * @brief Resets the internal index.
     *
     * @param start_index New numeric suffix for next generated name.
     */
    void reset(std::uint64_t start_index = 1) noexcept {
        next_index_ = start_index;
    }

private:
    std::string compose(std::uint64_t index) const {
        if (prefix_.empty()) {
            return std::to_string(index);
        }
        const char tail = prefix_.back();
        const bool has_separator = (tail == '_' || tail == '-');
        return has_separator ? (prefix_ + std::to_string(index)) : (prefix_ + "_" + std::to_string(index));
    }

    std::string prefix_;
    std::uint64_t next_index_{1};
};

}  // namespace skymapf::utils
