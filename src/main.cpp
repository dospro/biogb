#include<iostream>
#include"cCpu.h"

int main(int argc, char *argv[]) {
    cCpu *gb;
    std::cout << "BioGB v5.0\n";
    if (argc < 2) {
        std::cout << "Usage: biogb game.gb\n";
        return 0;
    }
    std::string rom_file_name{argv[1]};
    gb = new cCpu;
    if (gb->init_cpu(rom_file_name)) {
        while (gb->isCpuRunning())
            gb->doCycle();
    }
    delete gb;
    return 0;
}
