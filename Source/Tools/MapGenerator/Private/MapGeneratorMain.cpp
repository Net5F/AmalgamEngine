#include "Ignore.h"
#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char* argv[])
{
    AM::ignore(argc);
    AM::ignore(argv);

    std::cout << "Hi" << std::endl;

    return 0;
}
