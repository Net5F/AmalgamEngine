#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[])
{
    /* Run Tests */
    // Tests are ran on the first enumerated device.
    int result = Catch::Session().run(argc, argv);

    return result;
}
