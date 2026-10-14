#include "VirtualScreen.h"

namespace sn
{
#ifdef __EMSCRIPTEN__

void VirtualScreen::create(unsigned int w, unsigned int h, float, Color color)
{
    m_width  = w;
    m_height = h;
    m_pixels.assign(static_cast<std::size_t>(w) * h * 4, 0);
    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
            setPixel(x, y, color);
    }
}

void VirtualScreen::setPixel(std::size_t x, std::size_t y, Color color)
{
    if (x >= m_width || y >= m_height)
        return;
    const std::size_t index = (y * m_width + x) * 4;
    m_pixels[index]         = color.r;
    m_pixels[index + 1]     = color.g;
    m_pixels[index + 2]     = color.b;
    m_pixels[index + 3]     = color.a;
}

const std::uint8_t* VirtualScreen::pixels() const
{
    return m_pixels.data();
}

#else

void VirtualScreen::create(unsigned int w, unsigned int h, float pixel_size, Color color)
{
    m_vertices.resize(w * h * 6);
    m_screenSize = { w, h };
    m_vertices.setPrimitiveType(sf::Triangles);
    m_pixelSize = pixel_size;
    for (std::size_t x = 0; x < w; ++x)
    {
        for (std::size_t y = 0; y < h; ++y)
        {
            auto         index = (x * m_screenSize.y + y) * 6;
            sf::Vector2f coord2d(x * m_pixelSize, y * m_pixelSize);
            sf::Color    fill(color.r, color.g, color.b, color.a);

            m_vertices[index].position     = coord2d;
            m_vertices[index].color        = fill;

            m_vertices[index + 1].position = coord2d + sf::Vector2f { m_pixelSize, 0 };
            m_vertices[index + 1].color    = fill;

            m_vertices[index + 2].position = coord2d + sf::Vector2f { m_pixelSize, m_pixelSize };
            m_vertices[index + 2].color    = fill;

            m_vertices[index + 3].position = coord2d + sf::Vector2f { m_pixelSize, m_pixelSize };
            m_vertices[index + 3].color    = fill;

            m_vertices[index + 4].position = coord2d + sf::Vector2f { 0, m_pixelSize };
            m_vertices[index + 4].color    = fill;

            m_vertices[index + 5].position = coord2d;
            m_vertices[index + 5].color    = fill;
        }
    }
}

void VirtualScreen::setPixel(std::size_t x, std::size_t y, Color color)
{
    auto index = (x * m_screenSize.y + y) * 6;
    if (index >= m_vertices.getVertexCount())
        return;

    sf::Color fill(color.r, color.g, color.b, color.a);
    m_vertices[index].color     = fill;
    m_vertices[index + 1].color = fill;
    m_vertices[index + 2].color = fill;
    m_vertices[index + 3].color = fill;
    m_vertices[index + 4].color = fill;
    m_vertices[index + 5].color = fill;
}

void VirtualScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_vertices, states);
}

#endif
}
