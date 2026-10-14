#pragma once
#include <cstdint>

namespace sn
{
struct Color
{
    std::uint8_t r, g, b, a;

    constexpr Color(std::uint8_t red = 0, std::uint8_t green = 0, std::uint8_t blue = 0, std::uint8_t alpha = 255)
      : r(red)
      , g(green)
      , b(blue)
      , a(alpha)
    {
    }

    explicit constexpr Color(std::uint32_t rgba)
      : r(static_cast<std::uint8_t>((rgba >> 24) & 0xff))
      , g(static_cast<std::uint8_t>((rgba >> 16) & 0xff))
      , b(static_cast<std::uint8_t>((rgba >> 8) & 0xff))
      , a(static_cast<std::uint8_t>(rgba & 0xff))
    {
    }
};
}
