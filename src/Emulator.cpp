#include "Emulator.h"
#include "APU/Constants.h"
#include "KeyInput.h"
#include "Keys.h"
#include "Log.h"

#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace sn
{
using std::chrono::high_resolution_clock;

Emulator::Emulator()
  : m_cpu(m_bus)
  , m_audioPlayer(static_cast<int>(1.0 / apu_clock_period_s.count()))
  , m_ppu(m_pictureBus, m_emulatorScreen)
  , m_apu(m_audioPlayer, m_cpu.createIRQHandler(), [&](Address addr) { return DMCDMA(addr); })
  , m_bus(m_ppu, m_apu, m_controller1, m_controller2, [&](Byte b) { OAMDMA(b); })
  , m_screenScale(3.f)
  , m_lastWakeup()
{
    m_ppu.setInterruptCallback([&]() { m_cpu.nmiInterrupt(); });
}

bool Emulator::bootFromFile(const std::string& rom_path)
{
    if (!m_cartridge.loadFromFile(rom_path))
        return false;
    return finishBoot();
}

bool Emulator::bootFromMemory(const std::uint8_t* data, std::size_t size)
{
    if (!m_cartridge.loadFromMemory(data, size))
        return false;
    return finishBoot();
}

bool Emulator::finishBoot()
{
    m_mapper = Mapper::createMapper(static_cast<Mapper::Type>(m_cartridge.getMapper()),
                                    m_cartridge,
                                    m_cpu.createIRQHandler(),
                                    [&]() { m_pictureBus.updateMirroring(); });
    if (!m_mapper)
    {
        LOG(Error) << "Creating Mapper failed. Probably unsupported." << std::endl;
        return false;
    }

    if (!m_bus.setMapper(m_mapper.get()) || !m_pictureBus.setMapper(m_mapper.get()))
        return false;

    m_cpu.reset();
    m_ppu.reset();

#ifndef __EMSCRIPTEN__
    m_window.create(sf::VideoMode(NESVideoWidth * m_screenScale, NESVideoHeight * m_screenScale),
                    "SimpleNES",
                    sf::Style::Titlebar | sf::Style::Close);
    m_window.setVerticalSyncEnabled(true);
#endif
    m_emulatorScreen.create(NESVideoWidth, NESVideoHeight, m_screenScale, Color(255, 255, 255));

    m_lastWakeup  = high_resolution_clock::now();
    m_elapsedTime = m_lastWakeup - m_lastWakeup;

    m_audioPlayer.start();
    return true;
}

void Emulator::stepMachine()
{
    m_ppu.step();
    m_ppu.step();
    m_ppu.step();
    m_cpu.step();
    m_apu.step();
}

void Emulator::advanceTime()
{
    const auto now = high_resolution_clock::now();
    m_elapsedTime += now - m_lastWakeup;
    m_lastWakeup   = now;

#ifdef __EMSCRIPTEN__
    int             steps = 0;
    constexpr int   maxSteps = 29781 * 2;
    while (m_elapsedTime > cpu_clock_period_ns && steps < maxSteps)
    {
        stepMachine();
        m_elapsedTime -= cpu_clock_period_ns;
        ++steps;
    }
    if (steps >= maxSteps)
        m_elapsedTime = Duration::zero();
#else
    while (m_elapsedTime > cpu_clock_period_ns)
    {
        stepMachine();
        m_elapsedTime -= cpu_clock_period_ns;
    }
#endif
}

void Emulator::present()
{
#ifdef __EMSCRIPTEN__
    EM_ASM(
      {
          if (typeof Module.nesPresent === 'function')
              Module.nesPresent($0, $1, $2);
      },
      m_emulatorScreen.pixels(),
      NESVideoWidth,
      NESVideoHeight);
#else
    m_window.draw(m_emulatorScreen);
    m_window.display();
#endif
}

void Emulator::tick()
{
#ifdef __EMSCRIPTEN__
    const bool f2 = isKeyDown(KeyF2);
    if (f2 && !m_prevF2)
    {
        m_pause = !m_pause;
        if (!m_pause)
            m_lastWakeup = high_resolution_clock::now();
    }
    m_prevF2 = f2;

    const bool f3 = isKeyDown(KeyF3);
    if (m_pause && f3 && !m_prevF3)
    {
        for (int i = 0; i < 29781; ++i)
            stepMachine();
        present();
    }
    m_prevF3 = f3;

    if (isKeyDown(KeyF4))
        Log::get().setLevel(Info);
    if (isKeyDown(KeyF5))
        Log::get().setLevel(InfoVerbose);

    if (!m_pause)
    {
        advanceTime();
        present();
    }
#endif
}

void Emulator::run(std::string rom_path)
{
    if (!bootFromFile(rom_path))
        return;

#ifndef __EMSCRIPTEN__
    sf::Event event;
    bool      focus = true, pause = false;
    while (m_window.isOpen())
    {
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
            {
                m_window.close();
                return;
            }
            else if (event.type == sf::Event::GainedFocus)
            {
                focus          = true;
                const auto now = high_resolution_clock::now();
                LOG(Info) << "Gained focus. Removing " << (now - m_lastWakeup).count() << "ns from timers" << std::endl;
                m_lastWakeup = now;
            }
            else if (event.type == sf::Event::LostFocus)
            {
                focus = false;
                LOG(Info) << "Losing focus; paused." << std::endl;
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2)
            {
                pause = !pause;
                if (!pause)
                {
                    const auto now = high_resolution_clock::now();
                    LOG(Info) << "Unpaused. Removing " << (now - m_lastWakeup).count() << "ns from timers" << std::endl;
                    m_lastWakeup = now;
                }
                else
                {
                    LOG(Info) << "Paused." << std::endl;
                }
            }
            else if (pause && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F3)
            {
                for (int i = 0; i < 29781; ++i)
                    stepMachine();
            }
            else if (focus && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F4)
            {
                Log::get().setLevel(Info);
            }
            else if (focus && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F5)
            {
                Log::get().setLevel(InfoVerbose);
            }
        }

        if (focus && !pause)
        {
            advanceTime();
            present();
        }
        else
        {
            sf::sleep(sf::milliseconds(1000 / 60));
        }
    }
#else
    (void)rom_path;
#endif
}

void Emulator::OAMDMA(Byte page)
{
    m_cpu.skipOAMDMACycles();
    auto page_ptr = m_bus.getPagePtr(page);
    if (page_ptr != nullptr)
    {
        m_ppu.doDMA(page_ptr);
    }
    else
    {
        LOG(Error) << "Can't get pageptr for DMA" << std::endl;
    }
}

Byte Emulator::DMCDMA(Address addr)
{
    m_cpu.skipDMCDMACycles();
    return m_bus.read(addr);
};

void Emulator::setVideoHeight(int height)
{
    m_screenScale = height / float(NESVideoHeight);
    LOG(Info) << "Scale: " << m_screenScale << " set. Screen: " << int(NESVideoWidth * m_screenScale) << "x"
              << int(NESVideoHeight * m_screenScale) << std::endl;
}

void Emulator::setVideoWidth(int width)
{
    m_screenScale = width / float(NESVideoWidth);
    LOG(Info) << "Scale: " << m_screenScale << " set. Screen: " << int(NESVideoWidth * m_screenScale) << "x"
              << int(NESVideoHeight * m_screenScale) << std::endl;
}
void Emulator::setVideoScale(float scale)
{
    m_screenScale = scale;
    LOG(Info) << "Scale: " << m_screenScale << " set. Screen: " << int(NESVideoWidth * m_screenScale) << "x"
              << int(NESVideoHeight * m_screenScale) << std::endl;
}

void Emulator::setKeys(std::vector<int>& p1, std::vector<int>& p2)
{
    m_controller1.setKeyBindings(p1);
    m_controller2.setKeyBindings(p2);
}

void Emulator::muteAudio()
{
    m_audioPlayer.mute();
}

}
