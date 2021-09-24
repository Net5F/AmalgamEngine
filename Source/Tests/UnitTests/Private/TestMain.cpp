#include "catch2/catch_all.hpp"

int main(int argc, char* argv[])
{
    /* Run Tests */
    int result = Catch::Session().run(argc, argv);

    return result;
}
