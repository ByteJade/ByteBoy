#include <SDL2/SDL_audio.h>

struct Channel{
    bool active{false};
    float phase{0.0f};
    float duty{0.0f};
    uint8_t direction{0};

    uint16_t current_F{0};
    uint16_t lfsr{0};
    
    int sweep_counter{0};
    uint8_t sweep_timer{0};
    
    int envelope_counter{0};
    uint8_t envelope_volume{0};
    uint8_t envelope_period{0};

    int length_counter{0};
    uint8_t length_timer{0};
};

class MemoryMaster;
class APU{
public:
    uint8_t NR52;
    
    uint8_t NR10;
    uint8_t NR11;
    uint8_t NR12;
    uint8_t NR13;
    uint8_t NR14;

    uint8_t NR21;
    uint8_t NR22;
    uint8_t NR23;
    uint8_t NR24;

    uint8_t NR30;
    uint8_t NR31;
    uint8_t NR32;
    uint8_t NR33;
    uint8_t NR34;
    uint8_t NR41;
    uint8_t NR42;
    uint8_t NR43;
    uint8_t NR44;
    uint8_t NR50;
    uint8_t NR51;

    uint8_t wave_ram[16];
    SDL_AudioSpec spec;
    SDL_AudioDeviceID device;

    Channel SweepChannel;
    Channel BahChannel;
    Channel WaveChannel;
    Channel NoiseChannel;

    float leftVolume{0};
    float rightVolume{0};
    
    APU();
    ~APU();
    void updateEvelope(Channel& ch);
    void updateLenght(Channel& ch);
    void updateSweep(Channel& ch);
    
    bool write(uint16_t addr, uint8_t data);
    bool read(uint16_t addr, uint8_t& data);
};