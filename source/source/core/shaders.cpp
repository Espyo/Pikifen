/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Shader related functions.
 */

#include "game.h"
#include "misc_functions.h"
#include "shaders.h"


/**
 * @brief Compiles all shaders from their source.
 */
void ShaderManager::compileShaders() {
    //Liquid.
    compiledShaders[SHADER_TYPE_LIQUID] =
        al_create_shader(ALLEGRO_SHADER_GLSL);
        
    tryAttachShader(
        compiledShaders[SHADER_TYPE_LIQUID],
        ALLEGRO_PIXEL_SHADER, SHADER_SOURCE_FILES::LIQUID_FRAG_SHADER
    );
    tryAttachShader(
        compiledShaders[SHADER_TYPE_LIQUID],
        ALLEGRO_VERTEX_SHADER, SHADER_SOURCE_FILES::DEFAULT_VERT_SHADER
    );
    if(!al_build_shader(compiledShaders[SHADER_TYPE_LIQUID])) {
        al_destroy_shader(compiledShaders[SHADER_TYPE_LIQUID]);
        compiledShaders[SHADER_TYPE_LIQUID] = nullptr;
    };
    
    //Onion menu background.
    compiledShaders[SHADER_TYPE_ONION] =
        al_create_shader(ALLEGRO_SHADER_GLSL);
        
    tryAttachShader(
        compiledShaders[SHADER_TYPE_ONION],
        ALLEGRO_PIXEL_SHADER, SHADER_SOURCE_FILES::ONION_FRAG_SHADER
    );
    tryAttachShader(
        compiledShaders[SHADER_TYPE_ONION],
        ALLEGRO_VERTEX_SHADER, SHADER_SOURCE_FILES::DEFAULT_VERT_SHADER
    );
    if(!al_build_shader(compiledShaders[SHADER_TYPE_ONION])) {
        al_destroy_shader(compiledShaders[SHADER_TYPE_ONION]);
        compiledShaders[SHADER_TYPE_ONION] = nullptr;
    };
    
}


/**
 * @brief Returns a compiled shader.
 *
 * @param shaderType Type of shader.
 * @return The shader.
 */
ALLEGRO_SHADER* ShaderManager::getShader(SHADER_TYPE shaderType) {
    assert(shaderType < N_SHADER_TYPES);
    
    return compiledShaders[(int) shaderType];
}


/**
 * @brief Tries to attach shader code to a shader. Crashes the engine if it
 * fails unless compatibility mode is enabled.
 *
 * @param shader Shader to attach to.
 * @param type Allegro shader type.
 * @param source Shader source code.
 */
void ShaderManager::tryAttachShader(
    ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* source
) {
    if(!al_attach_shader_source(shader, type, source)) {
        if(!game.options.advanced.shaderCompatMode) {
            crash(
                "Shader compilation failure "
                "(try enabling the shader compatibility mode option?)",
                al_get_shader_log(shader), 1
            );
        }
    }
}
