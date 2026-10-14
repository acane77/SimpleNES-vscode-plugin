#include "Emulator.h"
#include "Keys.h"
#include "Log.h"

#include <emscripten.h>
#include <vector>

namespace
{
sn::Emulator* g_emulator = nullptr;
bool          g_running  = false;
bool          g_keys[128] = {};
}

namespace sn
{
bool isKeyDown(int key)
{
    if (key < 0 || key >= 128)
        return false;
    return g_keys[key];
}
}

namespace
{
void frame(void* arg)
{
    static_cast<sn::Emulator*>(arg)->tick();
}
}

extern "C"
{
EMSCRIPTEN_KEEPALIVE
void nes_set_key(int key, int down)
{
    if (key < 0 || key >= 128)
        return;
    g_keys[key] = down != 0;
}

EMSCRIPTEN_KEEPALIVE
void nes_release_keys()
{
    for (int i = 0; i < 128; ++i)
        g_keys[i] = false;
}

EMSCRIPTEN_KEEPALIVE
int nes_load_rom(const std::uint8_t* data, int length)
{
    if (data == nullptr || length <= 0)
        return 1;

    if (g_running)
    {
        emscripten_cancel_main_loop();
        g_running = false;
    }
    delete g_emulator;
    g_emulator = new sn::Emulator();

    std::vector<int> p1 { sn::KeyJ, sn::KeyK, sn::KeyRShift, sn::KeyReturn, sn::KeyW, sn::KeyS, sn::KeyA, sn::KeyD };
    std::vector<int> p2 { sn::KeyNumpad5, sn::KeyNumpad6, sn::KeyNumpad8, sn::KeyNumpad9,
                          sn::KeyUp,      sn::KeyDown,    sn::KeyLeft,    sn::KeyRight };
    g_emulator->setKeys(p1, p2);
    g_emulator->setVideoScale(1.f);

    if (!g_emulator->bootFromMemory(data, static_cast<std::size_t>(length)))
    {
        delete g_emulator;
        g_emulator = nullptr;
        return 2;
    }

    emscripten_set_main_loop_arg(frame, g_emulator, 0, 0);
    g_running = true;
    return 0;
}
}
