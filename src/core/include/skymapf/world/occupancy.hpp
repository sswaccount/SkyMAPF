#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

class IOccupancyMap {
public:
    virtual ~IOccupancyMap() = default;

    virtual common::CellIndex cell_count() const noexcept = 0;
    virtual bool is_walkable(common::CellIndex index) const noexcept = 0;
    virtual void set_walkable(common::CellIndex index, bool walkable) = 0;
};

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

using OccupancyMapPtr = std::shared_ptr<IOccupancyMap>;

}  // namespace skymapf::world
