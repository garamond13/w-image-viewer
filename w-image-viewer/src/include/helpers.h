#pragma once

#include "pch.h"

// Math
//
//

// Safe floating point comparations.
//

inline constexpr double WIV_DBL_EPS = 1e-15;
inline constexpr float WIV_FLT_EPS = 1e-6f;

inline bool is_equal(double a, double b) noexcept
{
    return std::abs(a - b) < WIV_DBL_EPS;
}

inline bool is_equal(float a, float b) noexcept
{
    return std::abs(a - b) < WIV_FLT_EPS;
}

inline bool is_zero(double a) noexcept
{
    return std::abs(a) < WIV_DBL_EPS;
}

inline bool is_zero(float a) noexcept
{
    return std::abs(a) < WIV_FLT_EPS;
}

inline bool is_not_zero(double a) noexcept
{
    return std::abs(a) >= WIV_DBL_EPS;
}

inline bool is_not_zero(float a) noexcept
{
    return std::abs(a) >= WIV_FLT_EPS;
}

//

template<typename T>
constexpr T get_ratio(auto a, auto b) noexcept
{
    return static_cast<T>(a) / static_cast<T>(b);
}

// Returns fractional part.
template<std::floating_point T>
inline auto frac(T f) noexcept
{
    return f - std::floor(f);
}

inline constexpr auto sqr(auto a) noexcept
{
    return a * a;
}

inline constexpr auto cube(auto a) noexcept
{
    return a * a * a;
}

//
//

// Win32
//

// Get width from RECT.
template<typename T>
constexpr T rc_w(const RECT& rect) noexcept
{
    return static_cast<T>(rect.right - rect.left);
}

// Get height from RECT.
template<typename T>
constexpr T rc_h(const RECT& rect) noexcept
{
    return static_cast<T>(rect.bottom - rect.top);
}

// Reverse of AdjustWindowRectEx().
inline BOOL UnAdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
{
    RECT rect = {};
    BOOL result = AdjustWindowRectEx(&rect, dwStyle, bMenu, dwExStyle);
    if (result) {
        lpRect->left -= rect.left;
        lpRect->top -= rect.top;
        lpRect->right -= rect.right;
        lpRect->bottom -= rect.bottom;
    }
    return result;
}

// Non blocking.
// Ignores return value.
inline void wiv_message(std::wstring text, std::wstring caption = L"WIV - Message", UINT type = 0)
{
    std::thread t([=]() noexcept { MessageBoxW(nullptr, text.c_str(), caption.c_str(), type); });
    t.detach();
}

inline int wiv_message_blocking(std::wstring text, std::wstring caption = L"WIV - Message", UINT type = 0) noexcept
{
    return MessageBoxW(nullptr, text.c_str(), caption.c_str(), type);
}

//

// Convert string to value.
//

// Needed in c++20.
template<typename>
inline constexpr bool always_false_v = false;

inline void strtoval(const std::string& str, auto& val, size_t* idx = nullptr, [[maybe_unused]] int base = 10)
{
    if constexpr (std::is_same_v<decltype(val), bool&>)
        val = static_cast<bool>(std::stoi(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), char&>)
        val = static_cast<char>(std::stoi(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), short&>)
        val = static_cast<short>(std::stoi(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), int&>)
        val = std::stoi(str.data(), idx, base);
    else if constexpr (std::is_same_v<decltype(val), long&>)
        val = std::stol(str.data(), idx, base);
    else if constexpr (std::is_same_v<decltype(val), long long&>)
        val = std::stoll(str.data(), idx, base);
    else if constexpr (std::is_same_v<decltype(val), unsigned short&>)
        val = static_cast<unsigned short>(std::stoul(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), unsigned char&>)
        val = static_cast<unsigned char>(std::stoi(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), unsigned int&>)
        val = static_cast<unsigned int>(std::stoul(str.data(), idx, base));
    else if constexpr (std::is_same_v<decltype(val), unsigned long&>)
        val = std::stoul(str.data(), idx, base);
    else if constexpr (std::is_same_v<decltype(val), unsigned long long&>)
        val = std::stoull(str.data(), idx, base);
    else if constexpr (std::is_same_v<decltype(val), float&>)
        val = std::stof(str.data(), idx);
    else if constexpr (std::is_same_v<decltype(val), double&>)
        val = std::stod(str.data(), idx);
    else if constexpr (std::is_same_v<decltype(val), long double&>)
        val = std::stold(str.data(), idx);
    else
        static_assert(always_false_v<decltype(val)>, "strtoval faild");
}

//

// Workaround for string literal as template-argument.
template <size_t n>
struct Char_array
{
    constexpr Char_array(const char(&str)[n]) :
        val(std::to_array(str))
    {}

    const std::array<char, n> val;
};
