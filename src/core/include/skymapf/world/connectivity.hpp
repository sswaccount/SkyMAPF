#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

class WorldModel;

class IConnectivityPolicy {
public:
    virtual ~IConnectivityPolicy() = default;

    virtual std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const = 0;
};

class GridAdjacencyPolicy final : public IConnectivityPolicy {
public:
    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;
};

class ExplicitGraphConnectivityPolicy final : public IConnectivityPolicy {
public:
    void add_directed_edge(common::CellIndex from, common::CellIndex to);
    void add_bidirectional_edge(common::CellIndex a, common::CellIndex b);
    void clear() noexcept;

    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;

private:
    std::unordered_map<common::CellIndex, std::vector<common::CellIndex>> adjacency_;
};

using ConnectivityPolicyPtr = std::shared_ptr<IConnectivityPolicy>;

}  // namespace skymapf::world
