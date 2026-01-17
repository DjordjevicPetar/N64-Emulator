#include "n64_system.hpp"

namespace n64 {

void N64System::run()
{
    while (true) {
        cpu_.execute_next_instruction();
    }
}

}