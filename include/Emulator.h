#ifndef EMULATOR_H
#define EMULATOR_H
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#ifndef __EMSCRIPTEN__
#include <SFML/Graphics.hpp>
#endif

#include "APU/APU.h"
#include "AudioPlayer.h"
#include "CPU.h"
#include "Controller.h"
#include "MainBus.h"
#include "PPU.h"
#include "PictureBus.h"

namespace sn
{
using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration  = std::chrono::high_resolution_clock::duration;

const int NESVideoWidth  = ScanlineVisibleDots;
const int NESVideoHeight = VisibleScanlines;

class Emulator
{
public:
    Emulator();
    void run(std::string rom_path);
    bool bootFromFile(const std::string& rom_path);
    bool bootFromMemory(const std::uint8_t* data, std::size_t size);
    void tick();
    void setVideoWidth(int width);
    void setVideoHeight(int height);
    void setVideoScale(float scale);
    void setKeys(std::vector<int>& p1, std::vector<int>& p2);
    void muteAudio();

private:
    bool                    finishBoot();
    void                    stepMachine();
    void                    advanceTime();
    void                    present();
    void                    OAMDMA(Byte page);
    Byte                    DMCDMA(Address addr);

    CPU                     m_cpu;

    AudioPlayer             m_audioPlayer;

    PictureBus              m_pictureBus;
    PPU                     m_ppu;
    APU                     m_apu;
    Cartridge               m_cartridge;
    std::unique_ptr<Mapper> m_mapper;

    Controller              m_controller1, m_controller2;

    MainBus                 m_bus;

#ifndef __EMSCRIPTEN__
    sf::RenderWindow        m_window;
#endif
    VirtualScreen           m_emulatorScreen;
    float                   m_screenScale;

    TimePoint               m_lastWakeup;

    Duration                m_elapsedTime;

#ifdef __EMSCRIPTEN__
    bool                    m_pause  = false;
    bool                    m_prevF2 = false;
    bool                    m_prevF3 = false;
#endif
};
}
#endif // EMULATOR_H
