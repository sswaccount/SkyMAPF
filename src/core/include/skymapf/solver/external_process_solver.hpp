/**
 * @file external_process_solver.hpp
 * @brief Declares a protocol-v1 adapter for out-of-process solvers.
 */
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "skymapf/protocol/v1_codec.hpp"
#include "skymapf/solver/i_solver.hpp"

namespace skymapf::solver {

struct ExternalProcessSolverConfig {
    std::string executable;
    std::vector<std::string> arguments;
    SolverInfo solver_info{
        "external_process",
        SolverFamily::Unknown,
        "unknown",
        "SkyMAPF protocol v1 external solver",
        {}
    };
    protocol::ObjectiveV1 objective{protocol::ObjectiveV1::SumOfCosts};
    std::size_t max_output_bytes{16U * 1024U * 1024U};
};

/**
 * @brief Runs one external executable using JSON over stdin/stdout.
 *
 * The child receives one `skymapf.solve-request.v1` document on stdin and must
 * emit one `skymapf.raw-solve-result.v1` document on stdout. Stderr is reserved
 * for diagnostics. On POSIX platforms the adapter enforces a hard wall timeout
 * and terminates the child process group when it expires.
 */
class ExternalProcessSolver final : public ISolver {
public:
    using ISolver::solve;

    explicit ExternalProcessSolver(ExternalProcessSolverConfig config);

    SolverInfo info() const override;

    SolveResult solve(
        const SolveInstance& instance,
        const SolveOptions& options
    ) override;

private:
    ExternalProcessSolverConfig config_;
};

}  // namespace skymapf::solver
