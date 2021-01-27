/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Program start and main loop.
 */

#include "game.h"


/* ----------------------------------------------------------------------------
 * Main function. It calls the game class's functions to initialize and run
 * the game.
 * argc:
 *   Command line argument count. Unused.
 * argv:
 *   Command line argument values. Unused.
 */
int main(int argc, char** argv) {
    game = game_class();
    
    int game_start_result = game.start();
    if(game_start_result != 0) {
        return game_start_result;
    }
    
    game.main_loop();
    
    game.shutdown();
    
    return 0;
}
