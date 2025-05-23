#pragma once

template<typename T>
struct Dims
{
    template<typename U>
    constexpr U get_width() const noexcept
    {
        return static_cast<U>(width);
    }

    template<typename U>
    constexpr U get_height() const noexcept
    {
        return static_cast<T>(height);
    }

    T width;
    T height;
};