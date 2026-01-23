#include <Game/MainApp.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {

    try 
    {
        MainApp game;
        
        game.run();
    }
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
