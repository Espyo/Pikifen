/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

#include "core/game.h"


/**
 * @brief Main function. It calls the game class's functions to initialize
 * and run the game.
 *
 * @param argc Command line argument count. Unused.
 * @param argv Command line argument values. Unused.
 * @return 0 if everything went well, or an error number otherwise.
 */
int main(int argc, char** argv) {
    int gameStartResult = game.start();
    if(gameStartResult != 0) {
        return gameStartResult;
    }
    
    game.mainLoop();
    
    game.shutdown();
    
    return 0;
}
