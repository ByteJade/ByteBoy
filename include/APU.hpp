#include <SDL2/SDL_audio.h>

struct Channel{
    bool active = false;
    float phase = 0.0f;
    uint16_t current_F = 0;
    uint16_t lfsr = 0;
    int counter = 0;
    
    int envelope_counter = 0;
    int envelope_volume= 0;
    int envelope_period= 0;

    int length_counter;
    int length_timer;
};

class MemoryMaster;
class APU{
public:

    uint8_t& NR52;
    
    uint8_t& NR10;
    uint8_t& NR11;
    uint8_t& NR12;
    uint8_t& NR13;
    uint8_t& NR14;

    uint8_t& NR21;
    uint8_t& NR22;
    uint8_t& NR23;
    uint8_t& NR24;


    uint8_t& NR30;
    uint8_t& NR31;
    uint8_t& NR32;
    uint8_t& NR33;
    uint8_t& NR34;
    uint8_t& NR41;
    uint8_t& NR42;
    uint8_t& NR43;
    uint8_t& NR44;
    uint8_t& NR50;
    uint8_t& NR51;

    uint8_t* wave_ram;
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device;

    Channel SweepChannel;
    Channel BahChannel;
    Channel WaveChannel;
    Channel NoiseChannel;

    int timer;
    
    APU(MemoryMaster& master);
    ~APU();
    float getDuty(uint8_t duty);
    void updateEvelope(Channel& ch, bool dir);
    void updateLenght(Channel& ch);
    void updateSweep(Channel& ch);
    void step();
};