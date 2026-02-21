#include <SDL2/SDL.h>
#include <cmath>

#include "../include/APU.hpp"
#include "../include/MEM.hpp"

static constexpr float SAMPLE_RATE = 48000;
static constexpr int AMPLITUDE = 28000;

static const uint8_t divisor_table[8] = {8, 16, 32, 48, 64, 80, 96, 112};
static const float duty_table[4] = {0.125f, 0.250f, 0.500f, 0.750f};

static void audioCallback(void* userdata, Uint8* stream, int len) {
    APU* self = static_cast<APU*>(userdata);
    
    Channel& ch1 = self->SweepChannel;
    Channel& ch2 = self->BahChannel;
    Channel& ch3 = self->WaveChannel;
    Channel& ch4 = self->NoiseChannel;
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int samples = len / sizeof(Sint16);

    uint8_t clock_shift = (self->NR43 >> 4) & 0xF;
    uint8_t counter_step = (self->NR43 >> 3) & 0x1;
    uint8_t dividing_ratio = self->NR43 & 0x7;

    float duty_cycle1 = duty_table[(self->NR11 >> 6) & 3];
    float duty_cycle2 = duty_table[(self->NR21 >> 6) & 3];

    bool dir1 = (self->NR12 >> 3) & 1;
    bool dir2 = (self->NR22 >> 3) & 1;
    bool dir4 = (self->NR42 >> 3) & 1;


    float left_volume = (self->NR50 & 0x7) / 7.0f;
    float right_volume = ((self->NR50 >> 4) & 0x7) / 7.0f;

    for (int i = 0; i < samples; i+=2) {
        float mixed_left = 0.f;
        float mixed_right = 0.f;
        if (ch1.active) {
            float frequency = 131072.0f / (2048.0f - ch1.current_F);
            
            float sample = (fmod(ch1.phase, 1.0f) < duty_cycle1) ? 1.0f : 0.0f;
            
            sample *= (ch1.envelope_volume / 15.0f);
            
            if (self->NR51 & 0x10) {
                mixed_left += sample * 0.25f;
            }
            if (self->NR51 & 0x01) {
                mixed_right += sample * 0.25f;
            }
            
            ch1.phase += frequency / SAMPLE_RATE;
            if (ch1.phase >= 1.0f) ch1.phase -= 1.0f;
            
            self->updateSweep(ch1);
            
            self->updateEvelope(ch1, dir1);
            self->updateLenght(ch1);
        }
        
        if (ch2.active) {
            float frequency = 131072.0f / (2048.0f - ch2.current_F);
            
            float sample = (fmod(ch2.phase, 1.0f) < duty_cycle2) ? 1.0f : 0.0f;

            sample *= (ch2.envelope_volume / 15.0f);
            
            if (self->NR51 & 0x20) {
                mixed_left += sample * 0.25f;
            }
            if (self->NR51 & 0x02) {
                mixed_right += sample * 0.25f;
            }

            ch2.phase += frequency / SAMPLE_RATE;
            if (ch2.phase >= 1.0f) ch2.phase -= 1.0f;
            
            self->updateEvelope(ch2, dir2);
            self->updateLenght(ch2);
        }
        bool dac_enabled = (self->NR30 >> 7) & 0x01;
        if (ch3.active && dac_enabled) {
            float frequency = 131072.0f / (2048.0f - ch3.current_F);
            
            ch3.phase += frequency / SAMPLE_RATE;
            if (ch3.phase >= 1.0f) ch3.phase -= 1.0f;
            
            int sample_index = (int)(ch3.phase * 64.0f) % 64;
            
            uint8_t wave_byte = self->wave_ram[sample_index / 2];
            uint8_t sample_4bit;
            
            if (sample_index % 2 == 0) {
                sample_4bit = (wave_byte >> 4) & 0x0F;
            } else {
                sample_4bit = wave_byte & 0x0F;
            }
            
            float sample = (float)sample_4bit / 15.0f;
            
            float gain = 0.f;
            switch (ch3.envelope_volume) {
                case 0: gain = 0.0f; break;
                case 1: gain = 1.0f; break;
                case 2: gain = 0.5f; break;
                case 3: gain = 0.25f; break;
            }
            
            sample *= gain;
            
            if (self->NR51 & 0x40) {
                mixed_left += (sample * 0.25f) / 4.f;
            }
            if (self->NR51 & 0x04) {
                mixed_right += (sample * 0.25f) / 4.f;
            }
            
            self->updateLenght(ch3);
        }

        if (ch4.active) {
            uint8_t r = divisor_table[dividing_ratio];
            
            float lfsr_frequency = 524288.0f / (r * (1 << (clock_shift + 1)));
            
            ch4.phase += lfsr_frequency / SAMPLE_RATE;
            
            if (ch4.phase >= 1.0f) {
                ch4.phase -= 1.0f;
                uint16_t feedback = ((ch4.lfsr >> 0) & 1) ^ ((ch4.lfsr >> 1) & 1);
                
                if (counter_step == 0) {
                    ch4.lfsr = (ch4.lfsr >> 1) | (feedback << 14);
                } else {
                    ch4.lfsr = (ch4.lfsr >> 1) | (feedback << 6);
                    ch4.lfsr |= 0xFF80;
                }
            }
            float sample = (ch4.lfsr & 1) ? 1.0f : -1.0f;
            
            sample *= (ch4.envelope_volume / 15.0f);
            
            if (self->NR51 & 0x80) {
                mixed_left += (sample * 0.25f) / 4.f;
            }
            if (self->NR51 & 0x08) {
                mixed_right += (sample * 0.25f) / 4.f;
            }

            self->updateEvelope(ch4, dir4);
            self->updateLenght(ch4);
        }
        
        mixed_left = std::max(-1.0f, std::min(1.0f, mixed_left * left_volume));
        mixed_right = std::max(-1.0f, std::min(1.0f, mixed_right * right_volume));
        
        buffer[i] = static_cast<Sint16>(AMPLITUDE * mixed_left);
        buffer[i+1] = static_cast<Sint16>(AMPLITUDE * mixed_right);
    }
}
APU::APU(MemoryMaster& master) :
NR52(master.readIO(0xFF26)), NR10(master.readIO(0xFF10)),
NR11(master.readIO(0xFF11)), NR12(master.readIO(0xFF12)), 
NR13(master.readIO(0xFF13)), NR14(master.readIO(0xFF14)),
NR21(master.readIO(0xFF16)),
NR22(master.readIO(0xFF17)), NR23(master.readIO(0xFF18)),
NR24(master.readIO(0xFF19)), NR30(master.readIO(0xFF1A)),
NR31(master.readIO(0xFF1B)), NR32(master.readIO(0xFF1C)),
NR33(master.readIO(0xFF1D)), NR34(master.readIO(0xFF1E)),
NR41(master.readIO(0xFF20)),
NR42(master.readIO(0xFF21)), NR43(master.readIO(0xFF22)),
NR44(master.readIO(0xFF23)), NR50(master.readIO(0xFF24)),
NR51(master.readIO(0xFF25)), wave_ram(&master.readIO(0xFF30))
{
    SDL_Init(SDL_INIT_AUDIO);
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
    if (device != 0) {
        SDL_PauseAudioDevice(device, 1);
        SDL_Delay(10);
        SDL_CloseAudioDevice(device);
        device = 0;
    }
}
void APU::updateEvelope(Channel& ch, bool dir){
    ch.envelope_counter++;
    int envelope_period_samples = SAMPLE_RATE / 64;
    if (ch.envelope_counter >= envelope_period_samples) {
        ch.envelope_counter -= envelope_period_samples;
        if (ch.envelope_period > 0) {
            if (dir && ch.envelope_volume < 15) ch.envelope_volume++;
            else if (!dir && ch.envelope_volume > 0) ch.envelope_volume--;
        }
        if (ch.envelope_volume == 0 || ch.envelope_volume == 15) {
            ch.envelope_period = 0;
        }
    }
}
void APU::updateLenght(Channel& ch){
    ch.length_counter ++;
    
    int length_period_samples = SAMPLE_RATE / 256;
    if (ch.length_counter >= length_period_samples) {
        ch.length_counter -= length_period_samples;
        ch.length_timer--;
        if (ch.length_timer == 0) {
            ch.active = false;
        }
    }
}
void APU::updateSweep(Channel& ch){
    uint8_t sweep_time = (NR10 >> 4) & 0x7;
    if (sweep_time == 0) return;

    ch.counter++;
    int samples_per_sweep = SAMPLE_RATE / 128;
    
    if (ch.counter >= samples_per_sweep*sweep_time) {
        ch.counter -= samples_per_sweep*sweep_time;
        
        uint8_t N = NR10 & 0x7;
        if (N > 0){ 
            uint16_t delta = ch.current_F >> N;
            uint8_t decreace = (NR10 >> 3) & 0x01;
            if (decreace) {
                if (delta >= ch.current_F) {
                    ch.active = false;
                } else {
                    ch.current_F -= delta;
                }
            } else {
                ch.current_F += delta;
                if (ch.current_F > 2047) {
                    ch.active = false;
                }
            }
        }
    }
}
void APU::step(){
    if ((NR52 >> 7)&1){
        //SDL_PauseAudioDevice(device, 0);
    }else{
        //SDL_PauseAudioDevice(device, 1);
        SweepChannel.active = false;
        BahChannel.active = false;
        WaveChannel.active = false;
        return;
    }
    if (NR14 & 0x80) {
        SweepChannel.active = true;

        SweepChannel.current_F = ((NR14 & 0x7) << 8) | NR13;
        SweepChannel.phase = 0.0f;
        SweepChannel.counter = 0;

        SweepChannel.envelope_counter = 0;
        SweepChannel.envelope_period = NR12 & 0x7;
        SweepChannel.envelope_volume = (NR12 >> 4) & 0x0F;

        SweepChannel.length_timer = 64 - (NR11 & 0x3F);
        if (!( (NR14 >> 6) & 1)) SweepChannel.length_timer = 128;
        SweepChannel.length_counter = 0;
        
        NR14 &= ~0x80;
    }
    if (NR24 & 0x80) {
        BahChannel.active = true;

        BahChannel.current_F = ((NR24 & 0x7) << 8) | NR23;
        BahChannel.phase = 0.0f;

        BahChannel.envelope_counter = 0;
        BahChannel.envelope_period = NR22 & 0x07;
        BahChannel.envelope_volume = (NR22 >> 4) & 0xF;
        
        BahChannel.length_timer = 64 - (NR21 & 0x3F);
        if (!((NR24 >> 6) & 1)) BahChannel.length_timer = 128;
        BahChannel.length_counter = 0;
        NR24 &= ~0x80;
    }
    if (NR34 & 0x80) {
        WaveChannel.active = true;
        
        WaveChannel.current_F = ((NR34 & 0x07) << 8) | NR33;
        WaveChannel.phase = 0.0f;
        
        
        WaveChannel.envelope_volume = (NR32 >> 5) & 3;

        WaveChannel.length_timer = 256 - NR31;
        if (!((NR34 >> 6) & 1)) WaveChannel.length_timer = 256;
        WaveChannel.length_counter = 0;
        
        
        NR34 &= ~0x80;
    }
    if (NR44 & 0x80) {
        NoiseChannel.active = true;
        
        uint8_t counter_step = (NR43 >> 3) & 1;
        if (counter_step == 0) {
            NoiseChannel.lfsr = 0x7FFF;
        } else {
            NoiseChannel.lfsr = 0x7F;
            NoiseChannel.lfsr |= 0xFF80;
        }
        
        NoiseChannel.counter = 0.0f;
        
        NoiseChannel.envelope_volume = (NR42 >> 4) & 0xF;
        NoiseChannel.envelope_period = NR42 & 0x7;
        NoiseChannel.envelope_counter = 0;
        
        NoiseChannel.length_timer = 64 - (NR41 & 0x3F);
        if (!((NR44 >> 6) & 1)) NoiseChannel.length_timer = 128;
        NoiseChannel.length_counter = 0;
        
        NR44 &= ~0x80;
    }
}
