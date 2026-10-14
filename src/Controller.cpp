#include "Controller.h"
#include "KeyInput.h"

namespace sn
{
Controller::Controller()
  : m_strobe(false)
  , m_keyStates(0)
  , m_keyBindings(TotalButtons)
{
}

void Controller::setKeyBindings(const std::vector<int>& keys)
{
    m_keyBindings = keys;
}

void Controller::strobe(Byte b)
{
    m_strobe = (b & 1);
    if (!m_strobe)
    {
        m_keyStates = 0;
        int shift   = 0;
        for (int button = A; button < TotalButtons; ++button)
        {
            m_keyStates |= (isKeyDown(m_keyBindings[static_cast<Buttons>(button)]) << shift);
            ++shift;
        }
    }
}

Byte Controller::read()
{
    Byte ret;
    if (m_strobe)
        ret = isKeyDown(m_keyBindings[A]);
    else
    {
        ret           = (m_keyStates & 1);
        m_keyStates >>= 1;
    }
    return ret | 0x40;
}

}

#ifndef __EMSCRIPTEN__
#include <SFML/Window/Keyboard.hpp>

namespace sn
{
bool isKeyDown(int key)
{
    if (key < 0)
        return false;
    return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key));
}
}
#endif
