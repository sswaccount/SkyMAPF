#include "skymapf/solver/external_process_solver.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <csignal>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace skymapf::solver {

namespace {

SolveResult error_result(std::string message) {
    SolveResult result;
    result.status = SolveStatus::Error;
    result.message = std::move(message);
    return result;
}

#if defined(__unix__) || defined(__APPLE__)

struct FileCloser {
    void operator()(std::FILE* file) const noexcept {
        if (file) {
            std::fclose(file);
        }
    }
};

using FilePtr = std::unique_ptr<std::FILE, FileCloser>;

FilePtr make_temporary_file() {
    return FilePtr{std::tmpfile()};
}

bool write_all(std::FILE* file, const std::string& content) {
    if (content.empty()) {
        return true;
    }
    return std::fwrite(content.data(), 1, content.size(), file) == content.size() &&
        std::fflush(file) == 0 && std::fseek(file, 0, SEEK_SET) == 0;
}

std::optional<std::string> read_limited(
    std::FILE* file,
    std::size_t limit,
    std::string* error_message
) {
    if (std::fflush(file) != 0 || std::fseek(file, 0, SEEK_SET) != 0) {
        *error_message = "failed to rewind external solver output";
        return std::nullopt;
    }
    std::string content;
    char buffer[4096];
    while (true) {
        const auto count = std::fread(buffer, 1, sizeof(buffer), file);
        if (count > 0) {
            if (content.size() > limit || count > limit - content.size()) {
                *error_message = "external solver output exceeded configured limit";
                return std::nullopt;
            }
            content.append(buffer, count);
        }
        if (count < sizeof(buffer)) {
            if (std::ferror(file)) {
                *error_message = "failed to read external solver output";
                return std::nullopt;
            }
            break;
        }
    }
    return content;
}

std::string with_stderr(std::string message, const std::string& stderr_text) {
    if (!stderr_text.empty()) {
        message += "; stderr: " + stderr_text;
    }
    return message;
}

void terminate_process_group(pid_t child) {
    if (::kill(-child, SIGKILL) != 0) {
        ::kill(child, SIGKILL);
    }
}

#endif

}  // namespace

ExternalProcessSolver::ExternalProcessSolver(ExternalProcessSolverConfig config)
    : config_(std::move(config)) {}

SolverInfo ExternalProcessSolver::info() const {
    return config_.solver_info;
}

SolveResult ExternalProcessSolver::solve(
    const SolveInstance& solve_instance,
    const SolveOptions& options
) {
    if (config_.executable.empty()) {
        return error_result("external solver executable must not be empty");
    }
    if (config_.max_output_bytes == 0) {
        return error_result("external solver output limit must be positive");
    }

    const auto problem_id = std::to_string(solve_instance.instance.id());
    std::string protocol_error;
    const auto request = protocol::ProtocolV1Codec::encode_solve_request(
        solve_instance.instance,
        options,
        problem_id,
        config_.objective,
        &protocol_error
    );
    if (!request) {
        SolveResult result;
        result.status = SolveStatus::Unsupported;
        result.message = std::move(protocol_error);
        return result;
    }

#if !defined(__unix__) && !defined(__APPLE__)
    SolveResult result;
    result.status = SolveStatus::Unsupported;
    result.message = "external process solver currently requires a POSIX platform";
    return result;
#else
    const std::string request_text = request->dump();
    auto input = make_temporary_file();
    auto output = make_temporary_file();
    auto diagnostics = make_temporary_file();
    if (!input || !output || !diagnostics) {
        return error_result("failed to create temporary streams for external solver");
    }
    if (!write_all(input.get(), request_text)) {
        return error_result("failed to prepare external solver request");
    }

    std::vector<std::string> command;
    command.reserve(config_.arguments.size() + 1);
    command.push_back(config_.executable);
    command.insert(command.end(), config_.arguments.begin(), config_.arguments.end());
    std::vector<char*> argv;
    argv.reserve(command.size() + 1);
    for (auto& argument : command) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    std::fflush(nullptr);
    const pid_t child = ::fork();
    if (child < 0) {
        return error_result(
            std::string{"failed to fork external solver: "} + std::strerror(errno)
        );
    }
    if (child == 0) {
        ::setpgid(0, 0);
        if (::dup2(::fileno(input.get()), STDIN_FILENO) < 0 ||
            ::dup2(::fileno(output.get()), STDOUT_FILENO) < 0 ||
            ::dup2(::fileno(diagnostics.get()), STDERR_FILENO) < 0) {
            ::_exit(126);
        }
        struct rlimit output_limit;
        output_limit.rlim_cur = static_cast<rlim_t>(config_.max_output_bytes);
        output_limit.rlim_max = static_cast<rlim_t>(config_.max_output_bytes);
        if (::setrlimit(RLIMIT_FSIZE, &output_limit) != 0) {
            ::_exit(126);
        }
        ::execvp(argv.front(), argv.data());
        ::_exit(127);
    }
    ::setpgid(child, child);

    const auto started = std::chrono::steady_clock::now();
    int wait_status = 0;
    bool timed_out = false;
    while (true) {
        const auto wait_result = ::waitpid(child, &wait_status, WNOHANG);
        if (wait_result == child) {
            break;
        }
        if (wait_result < 0 && errno != EINTR) {
            terminate_process_group(child);
            ::waitpid(child, &wait_status, 0);
            return error_result(
                std::string{"failed while waiting for external solver: "} +
                std::strerror(errno)
            );
        }
        if (options.time_limit_ms > 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started
            ).count();
            if (elapsed >= static_cast<std::int64_t>(options.time_limit_ms)) {
                timed_out = true;
                terminate_process_group(child);
                while (::waitpid(child, &wait_status, 0) < 0 && errno == EINTR) {}
                break;
            }
        }
        ::usleep(1000);
    }

    std::string read_error;
    const auto stderr_text = read_limited(
        diagnostics.get(),
        config_.max_output_bytes,
        &read_error
    );
    if (!stderr_text) {
        return error_result(std::move(read_error));
    }
    if (timed_out) {
        SolveResult result;
        result.status = SolveStatus::Timeout;
        result.message = with_stderr("external solver exceeded wall-time limit", *stderr_text);
        return result;
    }
    if (!WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0) {
        std::string message = WIFSIGNALED(wait_status)
            ? "external solver terminated by signal " + std::to_string(WTERMSIG(wait_status))
            : "external solver exited with code " + std::to_string(WEXITSTATUS(wait_status));
        return error_result(with_stderr(std::move(message), *stderr_text));
    }

    const auto stdout_text = read_limited(
        output.get(),
        config_.max_output_bytes,
        &read_error
    );
    if (!stdout_text) {
        return error_result(std::move(read_error));
    }
    try {
        const auto response = nlohmann::json::parse(*stdout_text);
        auto result = protocol::ProtocolV1Codec::decode_raw_solve_result(
            response,
            solve_instance.instance,
            problem_id,
            &protocol_error
        );
        if (!result) {
            return error_result(with_stderr(std::move(protocol_error), *stderr_text));
        }
        return std::move(*result);
    } catch (const std::exception& ex) {
        return error_result(with_stderr(
            std::string{"external solver returned invalid JSON: "} + ex.what(),
            *stderr_text
        ));
    }
#endif
}

}  // namespace skymapf::solver
