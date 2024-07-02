#pragma once

#include "pch.h"

// Helpers for benching execution time.
// bench_start() must be called first!

// DON'T USE THIS DIRECTLY!
inline std::chrono::high_resolution_clock::time_point bench_start_time;

inline void bench_start() noexcept
{
	bench_start_time = std::chrono::high_resolution_clock::now();
}

// Non blocking.
// The result will be in milliseconds (double).
inline void bench_end() noexcept
{
	const auto bench_end_time = std::chrono::high_resolution_clock::now();
	auto t = std::thread([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), L"Bench result", 0); });
	t.detach();
}

// Non blocking.
// The result will be in milliseconds (double).
inline void bench_end(const std::wstring what) noexcept
{
	const auto bench_end_time = std::chrono::high_resolution_clock::now();
	auto t = std::thread([=]() noexcept { MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), what.c_str(), 0); });
	t.detach();
}

// The result will be in milliseconds (double).
inline void bench_end_blocking() noexcept
{
	const auto bench_end_time = std::chrono::high_resolution_clock::now();
	MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), L"Bench result", 0);
}

// The result will be in milliseconds (double).
inline void bench_end_blocking(const std::wstring what) noexcept
{
	const auto bench_end_time = std::chrono::high_resolution_clock::now();
	MessageBoxW(nullptr, std::to_wstring(std::chrono::duration<double, std::chrono::milliseconds::period>(bench_end_time - bench_start_time).count()).c_str(), what.c_str(), 0);
}