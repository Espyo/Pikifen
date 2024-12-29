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

#include <epoxy/gl.h>
#include <epoxy/glx.h>

enum SHADER_TYPE {
    SHADER_TYPE_LIQUID,
    
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

    shader_manager();
    ALLEGRO_SHADER* get_shader(SHADER_TYPE shader_type);
    
    void compile_shaders();

};