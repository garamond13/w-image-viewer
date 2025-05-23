#pragma once

#include "pch.h"

// Helpers for benching execution time.
// Bench::start() must be called first!

struct Bench
{
    Bench() = delete;
    
    static inline void start() noexcept
    {
        bench_start_time = std::chrono::high_resolution_clock::now();
    }

    // Non blocking.
    // The result will be in milliseconds (double).
    static inline void end() noexcept
    {
        const auto bench_end_time = std::chrono::high_resolution_clock::now();
        auto t = std::thread([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), L"Bench result", 0); });
        t.detach();
    }

    // Non blocking.
    // The result will be in milliseconds (double).
    static inline void end(const std::wstring what) noexcept
    {
        const auto bench_end_time = std::chrono::high_resolution_clock::now();
        auto t = std::thread([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), what.c_str(), 0); });
        t.detach();
    }

    // The result will be in milliseconds (double).
    static inline void end_blocking() noexcept
    {
        const auto bench_end_time = std::chrono::high_resolution_clock::now();
        MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), L"Bench result", 0);
    }

    // The result will be in milliseconds (double).
    static inline void end_blocking(const std::wstring what) noexcept
    {
        const auto bench_end_time = std::chrono::high_resolution_clock::now();
        MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), what.c_str(), 0);
    }

private:
    static inline std::chrono::high_resolution_clock::time_point bench_start_time;
};
