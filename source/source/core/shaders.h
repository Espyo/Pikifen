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

namespace SHADER_SOURCE_FILES {
extern const char* DEFAULT_VERT_SHADER;
extern const char* LIQUID_FRAG_SHADER;
extern const char* ONION_FRAG_SHADER;
};


//Types of shaders.
enum SHADER_TYPE {

    //Liquid sectors, like bodies of water.
    SHADER_TYPE_LIQUID,
    
    //Onion swirls, used for the Onion menu's background.
    SHADER_TYPE_ONION,
    
    //Total number of shader types.
    N_SHADER_TYPES
    
};


/**
 * @brief Manages everything regarding shaders.
 */
struct ShaderManager {

    //--- Members ---
    
    //Array of compiled shaders,
    ALLEGRO_SHADER* compiledShaders[N_SHADER_TYPES];
    
    
    //--- Function declarations ---
    
    ALLEGRO_SHADER* getShader(SHADER_TYPE shaderType);
    void compileShaders();
    
    
    private:
    
    //--- Function declarations ---
    
    void tryAttachShader(
        ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* source
    );
    
};
