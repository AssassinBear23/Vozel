#include "game/Game.h"

using namespace game;

int main()
{
    Game game;
    
    if (!game.init("#version 430"))
    {
        return -1;
    }

    // Run the game (contains the main loop)
    game.run();

    // Cleanup happens automatically in Game destructor
    return 0;
}
