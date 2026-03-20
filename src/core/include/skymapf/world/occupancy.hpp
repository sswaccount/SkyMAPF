/**
 * @file occupancy.hpp
 * @brief Defines occupancy map interfaces used by world models.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

/**
 * @brief Abstracts walkability storage for world cells.
 *
 * Different implementations may optimize for dense, sparse, or dynamic maps.
 */
class IOccupancyMap {
public:
    virtual ~IOccupancyMap() = default;

    virtual common::CellIndex cell_count() const noexcept = 0;
    virtual bool is_walkable(common::CellIndex index) const noexcept = 0;
    virtual void set_walkable(common::CellIndex index, bool walkable) = 0;
};

/// Dense walkability storage implementation backed by a byte vector.
class DenseOccupancyMap final : public IOccupancyMap {
public:
    explicit DenseOccupancyMap(common::CellIndex cell_count, bool default_walkable = true)
        : walkable_(static_cast<std::size_t>(cell_count), default_walkable ? 1u : 0u) {}

    common::CellIndex cell_count() const noexcept override {
        return static_cast<common::CellIndex>(walkable_.size());
    }

    bool is_walkable(common::CellIndex index) const noexcept override {
        if (index >= cell_count()) {
            return false;
        }
        return walkable_[static_cast<std::size_t>(index)] != 0;
    }

    void set_walkable(common::CellIndex index, bool walkable) override {
        if (index >= cell_count()) {
            return;
        }
        walkable_[static_cast<std::size_t>(index)] = walkable ? 1u : 0u;
    }

private:
    std::vector<std::uint8_t> walkable_;
};

/// Shared ownership handle for occupancy map implementations.
using OccupancyMapPtr = std::shared_ptr<IOccupancyMap>;

}  // namespace skymapf::world
