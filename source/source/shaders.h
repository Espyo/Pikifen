/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for shader related functions.
 */

#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

/*
TODO: Add a OpenGL extension library when it'll compile on windows
#include <epoxy/gl.h>
#include <epoxy/glx.h>
*/

//Types of shaders.
enum SHADER_TYPE {

    //Liquid sectors, like bodies of water.
    SHADER_TYPE_LIQUID,
    
    //Total number of shader types.
    N_SHADER_TYPES
    
};


/**
 * @brief Manages everything regarding shaders.
 */
struct shader_manager {

    //--- Members ---
    
    //Array of compiled shaders,
    ALLEGRO_SHADER* compiled_shaders[N_SHADER_TYPES];
    
    
    //--- Function declarations ---
    
    ALLEGRO_SHADER* get_shader(SHADER_TYPE shader_type);
    void compile_shaders();
    
    
    private:
    
    //--- Function declarations ---
    
    void try_attach_shader(
        ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* source
    );
    
};
