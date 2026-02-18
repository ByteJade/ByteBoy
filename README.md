# ByteBoy
## GameBoy Color Emulator in C++
Small DMG/GBC emulator written in C++ with SDL2

### Completed:
- All games supported  
- MCB5 supported  
- Scanline-based PPU  
- Timer & interrupts  
- Save files  
- HDMA  
### WIP:  
- Correct APU sound - does not fully correspond to the original  
- MCB3/1 support - Basic banking, need accuracy fixes  
- RTC clock - a time latch needs to be made  
- Good code?  

## Accuracy
cpu_instrs (blargg): pass  
instr_timing (blargg): pass  
acid2 tests (mattcurrie): pass  
MCB5 tests (mooneye): pass  
MCB3/1 tests (mooneye): fail

## Controls
D-Pad - W A S D  
A - Q  
B - E  
Start - Z  
Select - X  
A+B+Start+Select - C  

