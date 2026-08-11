#include <iostream>
#include <string_view>

#include <skymapf/common/space.hpp>
#include <skymapf/protocol/v1_codec.hpp>
#include <skymapf/version.hpp>

int main() {
    const auto space = skymapf::common::SpaceSpec::make_2d(2, 2);
    if (!space.is_valid()) {
        return 1;
    }
    if (std::string_view{skymapf::protocol::kProblemProtocolV1} !=
        "skymapf.problem.v1") {
        return 2;
    }
    std::cout << "SkyMAPF " << skymapf::kVersionString << " installed SDK OK\n";
    return 0;
}
