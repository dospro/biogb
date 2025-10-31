#include <print>

#include "CPU/cpu.h"

int main(int argc, char *argv[]) {
    std::println("BioGB v5.0");
    if (argc < 2) {
        std::println(stderr, "Usage: biogb <game.gb>");
        return EXIT_FAILURE;
    }
    const std::string rom_file_name{argv[1]};

    cCpu gb;
    if (!gb.init_cpu(rom_file_name)) {
        std::println(stderr, "Initialization failed: {}", rom_file_name);
        return EXIT_FAILURE;
    }
    while (gb.isCpuRunning())
        gb.runFrame();
    return 0;
}
