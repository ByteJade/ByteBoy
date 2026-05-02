#include "../include/PPU.hpp"
#include "../include/CPU.hpp"
#include "../include/APU.hpp"
#include "../include/MEM.hpp"
#include "../include/Display.hpp"
#include "../include/Timer.hpp"
#include "../include/types.hpp"

class GameBoy{
public:
    InterruptState bus;
    Window context;
    MemoryMaster MEM;
    CPU GB;
    PPU GC;
    APU AP;
    Timer timer;

    GameBoy() : context(640,576,"gbc.emu"), GB(MEM),
    GC(MEM, context)
    {
        MEM.setTimer(&timer);
        MEM.setJoypad(&context.joypad);
        MEM.setPPU(&GC);
        MEM.setAPU(&AP);
    }
    
    void start(){
        int time;
        GB.init();
        context.joypad.update();
        while (context.isOpen()) {
            time = GB.step();
            GC.step(time);
            timer.step(time);
        }
    }
    void waitUntilDropFile(){
        while (!context.poolFile(MEM) && context.isOpen()) {
            context.show();
        }
    }
};
int main(int args, char *argv[]){
    GameBoy GB;

    if (args < 2){
        GB.waitUntilDropFile();
        if (GB.context.isOpen()) GB.start();
    }else{
        if (GB.MEM.readFromFile(argv[1])){
            GB.start();
        }
    }
    return 0;
}