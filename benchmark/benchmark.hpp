#pragma once
#include <chrono>

#include "configuration.hpp"
namespace benchmark {
inline volatile std::uint64_t sink = 0;
class benchmark {
   public:
    struct result {
        std::string name;
        double milliseconds{};

        [[nodiscard]]
        double seconds() const noexcept {
            return milliseconds / 1000.0;
        };
    };
    explicit benchmark(configuration config = {})
        : config_(config) {}

    virtual ~benchmark() = default;

    benchmark(const benchmark&) = delete;
    benchmark& operator=(const benchmark&) = delete;

    virtual void run() = 0;

   protected:
    configuration config_;

    static void print_header(std::string_view name) {
        std::cout
            << "\n============================================\n"
            << "        " << name << '\n'
            << "============================================\n";
    }

    template <typename Fn>
    static result measure(
        std::string_view name,
        Fn&& fn) {
        const auto start = clock::now();

        std::forward<Fn>(fn)();

        const auto end = clock::now();

        const double milliseconds =
            std::chrono::duration<double, std::milli>(
                end - start)
                .count();

        std::cout
            << std::left
            << std::setw(40)
            << name
            << std::right
            << std::setw(12)
            << std::fixed
            << std::setprecision(3)
            << milliseconds
            << " ms\n";

        return {
            std::string(name),
            milliseconds};
    }

    static void print_sink() {
        std::cout
            << "\nSink: "
            << sink
            << '\n';
    }
};
};  // namespace benchmark