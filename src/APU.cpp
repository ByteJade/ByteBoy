#include <SDL2/SDL.h>
#include <cmath>

#include "../include/APU.hpp"
#include "../include/MEM.hpp"

#define SAMPLE_RATE 48000
#define AMPLITUDE 28000
#define EVELOPE_RATE SAMPLE_RATE / 64
#define SWEEP_RATE SAMPLE_RATE / 128
#define LENGHT_RATE SAMPLE_RATE / 256

static const uint8_t divisor_table[8] = {8, 16, 32, 48, 64, 80, 96, 112};
static const float duty_table[4] = {0.125f, 0.250f, 0.500f, 0.750f};
static const float envelope_table[4] = {0.0f, 1.0f, 0.5f, 0.25f};

static void audioCallback(void* userdata, Uint8* stream, int len) {
    APU* self = static_cast<APU*>(userdata);
    
    Channel& ch1 = self->SweepChannel;
    Channel& ch2 = self->BahChannel;
    Channel& ch3 = self->WaveChannel;
    Channel& ch4 = self->NoiseChannel;
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int samples = len / sizeof(Sint16);

    uint8_t counter_step = (self->NR43 >> 3) & 1;
    float ch1F = (131072.0f / (2048.0f - ch1.current_F)) / SAMPLE_RATE;
    float ch2F = (131072.0f / (2048.0f - ch2.current_F)) / SAMPLE_RATE;
    float ch3F = (131072.0f / (2048.0f - ch3.current_F)) / SAMPLE_RATE;
    float ch4F = (4194304.0f / (ch4.current_F << (self->NR43 >> 4))) / SAMPLE_RATE;

    float vol1 = (ch1.envelope_volume / 15.0f) * 0.25f;
    float vol2 = (ch2.envelope_volume / 15.0f) * 0.25f;
    float vol3 = envelope_table[ch3.envelope_volume] * 0.2f;
    float vol4 = (ch4.envelope_volume / 15.0f) * 0.2f;

    bool dac_enabled = self->NR30 & 0x80;
    for (int i = 0; i < samples; i+=2) {
        float mixed_left = 0.f;
        float mixed_right = 0.f;
        if (ch1.active) {
            ch1.phase += ch1F;
            if (ch1.phase >= 1.0f) ch1.phase -= 1.0f;
            
            float sample = (fmod(ch1.phase, 1.0f) < ch1.duty) ? 1.0f : 0.0f;
            
            sample *= vol1;
            
            if (self->NR51 & 0x10) {
                mixed_left += sample;
            }
            if (self->NR51 & 0x01) {
                mixed_right += sample;
            }
            
            self->updateSweep(ch1);
            
            self->updateEvelope(ch1);
            self->updateLenght(ch1);
        }
        
        if (ch2.active) {
            ch2.phase += ch2F;
            if (ch2.phase >= 1.0f) ch2.phase -= 1.0f;

            float sample = (fmod(ch2.phase, 1.0f) < ch2.duty) ? 1.0f : 0.0f;

            sample *= vol2;
            
            if (self->NR51 & 0x20) {
                mixed_left += sample;
            }
            if (self->NR51 & 0x02) {
                mixed_right += sample;
            }
            
            self->updateEvelope(ch2);
            self->updateLenght(ch2);
        }
        if (ch3.active && dac_enabled) {
            ch3.phase += ch3F;
            if (ch3.phase >= 1.0f) ch3.phase -= 1.0f;
            
            int sample_index = (int)(ch3.phase * 32.0f) % 32;
            
            uint8_t wave_byte = self->wave_ram[sample_index / 2];
            uint8_t sample_4bit;
            
            if (sample_index % 2 == 0) {
                sample_4bit = wave_byte >> 4;
            } else {
                sample_4bit = wave_byte & 0x0F;
            }
            
            float sample = (float)sample_4bit / 15.0f;
            
            sample *= vol3;
            
            if (self->NR51 & 0x40) {
                mixed_left += sample;
            }
            if (self->NR51 & 0x04) {
                mixed_right += sample;
            }
            
            self->updateLenght(ch3);
        }

        if (ch4.active) {
            ch4.phase += ch4F;
            
            if (ch4.phase >= 1.0f) {
                ch4.phase -= 1.0f;
                uint16_t feedback = (ch4.lfsr & 1) ^ ((ch4.lfsr >> 1) & 1);
                ch4.lfsr >>= 1;
                ch4.lfsr |= feedback << 14;
                if (counter_step){
                    ch4.lfsr &= ~0x40; // reset bit 7
                    ch4.lfsr |= feedback << 6;
                }
            }
            float sample = -(~(ch4.lfsr) & 1) * vol4;
            
            if (self->NR51 & 0x80) {
                mixed_left += sample;
            }
            if (self->NR51 & 0x08) {
                mixed_right += sample;
            }

            self->updateEvelope(ch4);
            self->updateLenght(ch4);
        }
        buffer[i] = AMPLITUDE * mixed_left * self->leftVolume;
        buffer[i+1] = AMPLITUDE * mixed_right * self->rightVolume;
    }
}
APU::APU()
{
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 2048;
    spec.callback = audioCallback;
    spec.userdata = this;
    device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    SDL_PauseAudioDevice(device, 0);
}
APU::~APU(){
    if (device) SDL_CloseAudioDevice(device);
}
void APU::updateEvelope(Channel& ch){
    ch.envelope_counter++;
    if (ch.envelope_counter >= EVELOPE_RATE) {
        ch.envelope_counter -= EVELOPE_RATE;
        if (ch.envelope_period > 0) {
            if (ch.envelope_volume == 0 || ch.envelope_volume == 15) {
                ch.envelope_period = 0;
                return;
            }
            if (ch.direction) ch.envelope_volume++;
            else ch.envelope_volume--;
        }
    }
}
void APU::updateLenght(Channel& ch){
    ch.length_counter ++;
    
    if (ch.length_counter >= LENGHT_RATE) {
        ch.length_counter -= LENGHT_RATE;
        ch.length_timer--;
        if (ch.length_timer == 0) {
            ch.active = false;
        }
    }
}
void APU::updateSweep(Channel& ch){
    uint8_t sweep_time = (NR10 >> 4) & 0x7;
    if (sweep_time == 0) return;

    ch.sweep_counter++;
    
    if (ch.sweep_counter >= SWEEP_RATE*sweep_time) {
        ch.sweep_counter -= SWEEP_RATE*sweep_time;
        
        uint8_t N = NR10 & 0x7;
        uint16_t delta = ch.current_F >> N;
        uint8_t decreace = (NR10 >> 3) & 1;
        if (decreace) {
            if (delta >= ch.current_F) {
                ch.active = false;
            }
            ch.current_F -= delta;
        } else {
            ch.current_F += delta;
            if (ch.current_F > 2047) {
                ch.active = false;
            }
        }
    }
}

bool APU::write(uint16_t addr, uint8_t data){
    if (addr > 0xFF3F) return false;
    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        wave_ram[addr - 0xFF30] = data;
        return true;
    }
    switch (addr) {
        case (0xFF10):
            NR10 = data;
            return true;
        case (0xFF11):
            SweepChannel.duty = duty_table[data >> 6];
            SweepChannel.length_timer = 64 - (data & 0x3F);
            NR11 = data;
            return true;
        case (0xFF12):
            SweepChannel.direction = (data >> 3) & 1;
            SweepChannel.envelope_period = data & 0x7;
            SweepChannel.envelope_volume = data >> 4;
            NR12 = data;
            return true;
        case (0xFF13):
            NR13 = data;
            return true;
        case (0xFF14):
            if (data & 0x80){
                SweepChannel.current_F = ((data & 0x7) << 8) | NR13;
                SweepChannel.active = true;

                if (!(data & 0x40)) SweepChannel.length_timer = 64;

                SweepChannel.phase = 0.0f;
                SweepChannel.sweep_counter = 0;
                SweepChannel.envelope_counter = 0;
                SweepChannel.length_counter = 0;
            }
            NR14 = data & 0x7F;
            return true;
        case (0xFF16):
            BahChannel.duty = duty_table[data >> 6];
            BahChannel.length_timer = 64 - (data & 0x3F);
            NR21 = data;
            return true;
        case (0xFF17):
            BahChannel.direction = (data >> 3) & 1;
            BahChannel.envelope_period = data & 0x07;
            BahChannel.envelope_volume = data >> 4;
            NR22 = data;
            return true;
        case (0xFF18):
            NR23 = data;
            return true;
        case (0xFF19):
            if (data & 0x80){
                BahChannel.current_F = ((data & 0x7) << 8) | NR23;
                BahChannel.active = true;

                if (!(data & 0x40)) BahChannel.length_timer = 64;

                BahChannel.phase = 0.0f;
                BahChannel.envelope_counter = 0;
                BahChannel.length_counter = 0;
            }
            NR24 = data & 0x7F;
            return true;
        case (0xFF1A):
            NR30 = data;
            return true;
        case (0xFF1B):
            WaveChannel.length_timer = 255 - data;
            NR31 = data;
            return true;
        case (0xFF1C):
            WaveChannel.envelope_volume = data >> 5;
            NR32 = data;
            return true;
        case (0xFF1D):
            NR33 = data;
            return true;
        case (0xFF1E):
            if (data & 0x80){
                WaveChannel.current_F = ((data & 0x07) << 8) | NR33;
                WaveChannel.active = true;

                if (!(data & 0x40)) WaveChannel.length_timer = 255;

                WaveChannel.phase = 0.f;
                WaveChannel.length_counter = 0;
            }
            NR34 = data & 0x7F;
            return true;
        case (0xFF20):
            NoiseChannel.length_timer = 128 - (data & 0x3F);
            NR41 = data;
            return true;
        case (0xFF21):
            NoiseChannel.direction = (data >> 3) & 1;
            NoiseChannel.envelope_volume = (data >> 4) & 0xF;
            NoiseChannel.envelope_period = data & 0x7;
            NR42 = data;
            return true;
        case (0xFF22):
            NR43 = data;
            return true;
        case (0xFF23):
            if (data & 0x80){
                NoiseChannel.current_F = divisor_table[NR43 & 0x07];
                NoiseChannel.active = true;
                NoiseChannel.lfsr = 0x7FFF;
                
                if (!(data & 0x40)) NoiseChannel.length_timer = 128;
                
                NoiseChannel.phase = 0.f;
                NoiseChannel.envelope_counter = 0;
                NoiseChannel.length_counter = 0;
            }
            NR44 = data & 0x7F;
            return true;
        case (0xFF24):
            leftVolume = (data & 0x7) / 7.0f;
            rightVolume = ((data >> 4) & 0x7) / 7.0f;
            NR50 = data;
            return true;
        case (0xFF25):
            NR51 = data;
            return true;
        case (0xFF26):
            if (!(data & 0x80)){
                SweepChannel.active = false;
                BahChannel.active = false;
                WaveChannel.active = false;
            }
            NR52 = data;
            return true;
    }
    return false;
}
bool APU::read(uint16_t addr, uint8_t& data){
    if (addr > 0xFF3F) return false;
    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        data = wave_ram[addr - 0xFF30];
        return true;
    }
    switch (addr) {
        case (0xFF10):
            data = NR10;
            return true;
        case (0xFF11):
            data = NR11;
            return true;
        case (0xFF12):
            data = NR12;
            return true;
        case (0xFF13):
            data = NR13;
            return true;
        case (0xFF14):
            data = NR14;
            return true;
        case (0xFF16):
            data = NR21;
            return true;
        case (0xFF17):
            data = NR22;
            return true;
        case (0xFF18):
            data = NR23;
            return true;
        case (0xFF19):
            data = NR24;
            return true;
        case (0xFF1A):
            data = NR30;
            return true;
        case (0xFF1B):
            data = NR31;
            return true;
        case (0xFF1C):
            data = NR32;
            return true;
        case (0xFF1D):
            data = NR33;
            return true;
        case (0xFF1E):
            data = NR34;
            return true;
        case (0xFF20):
            data = NR41;
            return true;
        case (0xFF21):
            data = NR42;
            return true;
        case (0xFF22):
            data = NR43;
            return true;
        case (0xFF23):
            data = NR44;
            return true;
        case (0xFF24):
            data = NR50;
            return true;
        case (0xFF25):
            data = NR51;
            return true;
        case (0xFF26):
            data = NR52;
            return true;
    }
    return false;
}