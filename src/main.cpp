#include "n64_system.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file>" << std::endl;
        return 1;
    }

    n64::N64System n64_system(argv[1]);
    n64_system.run();
    return 0;
}
