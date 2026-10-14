#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H
#include "Color.h"

#ifdef __EMSCRIPTEN__
#include <cstdint>
#include <vector>
#else
#include <SFML/Graphics.hpp>
#endif

namespace sn
{
#ifdef __EMSCRIPTEN__
class VirtualScreen
{
public:
    void                create(unsigned int width, unsigned int height, float pixel_size, Color color);
    void                setPixel(std::size_t x, std::size_t y, Color color);
    const std::uint8_t* pixels() const;

private:
    unsigned int         m_width  = 0;
    unsigned int         m_height = 0;
    std::vector<std::uint8_t> m_pixels;
};
#else
class VirtualScreen : public sf::Drawable
{
public:
    void create(unsigned int width, unsigned int height, float pixel_size, Color color);
    void setPixel(std::size_t x, std::size_t y, Color color);

private:
    void         draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::Vector2u m_screenSize;
    float        m_pixelSize;
    sf::VertexArray m_vertices;
};
#endif
};
#endif // VIRTUALSCREEN_H
