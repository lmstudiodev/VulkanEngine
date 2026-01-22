#include <Prerequisite.h>

#include <Game/Game.h>

int main() {
    try 
    {
        Game game;
        
        game.run();
    }
    catch (...) 
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
