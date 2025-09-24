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
 * @brief Compiles a shader from its source.
 *
 * @param type Type of shader.
 * @param fragShaderSource GLSL source code for the fragment shader.
 * @param vertShaderSource GLSL source code for the vertex shader.
 */
void ShaderManager::compileShader(
    SHADER_TYPE type, const char* fragShaderSource, const char* vertShaderSource
) {
    compiledShaders[type] = al_create_shader(ALLEGRO_SHADER_GLSL);
    
    tryAttachShader(
        compiledShaders[type], ALLEGRO_PIXEL_SHADER, fragShaderSource
    );
    tryAttachShader(
        compiledShaders[type], ALLEGRO_VERTEX_SHADER, vertShaderSource
    );
    
    if(!al_build_shader(compiledShaders[type])) {
        al_destroy_shader(compiledShaders[type]);
        compiledShaders[type] = nullptr;
    };
}


/**
 * @brief Compiles all shaders from their source.
 */
void ShaderManager::compileShaders() {
    //Colorizer.
    compileShader(
        SHADER_TYPE_COLORIZER,
        SHADER_SOURCES::COLORIZER_FRAG_SHADER,
        SHADER_SOURCES::DEFAULT_VERT_SHADER
    );
    
    //Liquid.
    compileShader(
        SHADER_TYPE_LIQUID,
        SHADER_SOURCES::LIQUID_FRAG_SHADER,
        SHADER_SOURCES::DEFAULT_VERT_SHADER
    );
    
    //Onion menu background.
    compileShader(
        SHADER_TYPE_ONION,
        SHADER_SOURCES::ONION_FRAG_SHADER,
        SHADER_SOURCES::DEFAULT_VERT_SHADER
    );
    
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
