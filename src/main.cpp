#include "engine/app.hpp"
#include "vWindow.hpp"

#include <iostream>

int main()
{
    v::vApp app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}