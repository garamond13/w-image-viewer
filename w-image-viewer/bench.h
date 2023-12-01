#pragma once

#include "pch.h"

// Helpers for benching execution time.
// First call bench_start() than bench_end().

// DON'T USE THIS DIRECTLY!
// It should be optimized out in release build.
inline std::chrono::high_resolution_clock::time_point bench_start_time;

inline void bench_start() noexcept
{
	bench_start_time = std::chrono::high_resolution_clock::now();
}

// The result will be in milliseconds (double).
inline void bench_end() noexcept
{
	const auto bench_end_time{ std::chrono::high_resolution_clock::now() };
	std::thread t([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), L"Bench result", 0); });
	t.detach();
}

// The result will be in milliseconds (double).
inline void bench_end(const std::wstring what) noexcept
{
	const auto bench_end_time{ std::chrono::high_resolution_clock::now() };
	std::thread t([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), what.c_str(), 0); });
	t.detach();
}