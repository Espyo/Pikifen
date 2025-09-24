/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Shader source code.
 */

#include "shaders.h"


namespace SHADER_SOURCES {

#pragma region Default vertex shader

//Allegro default vertex shader.
const char* DEFAULT_VERT_SHADER = R"(

#version 130
in vec4 al_pos;
in vec4 al_color;
in vec2 al_texcoord;
uniform mat4 al_projview_matrix;
out vec4 varying_color;
out vec2 varying_texcoord;

void main()
{
varying_color = al_color;
varying_texcoord = al_texcoord;
gl_Position = al_projview_matrix * al_pos;
}

    )";


#pragma endregion
#pragma region Colorizer fragment shader

//Fragment shader for the colorizer.
const char* COLORIZER_FRAG_SHADER = R"(

#version 130

uniform sampler2D al_tex;
uniform bool al_use_tex;
uniform bool al_alpha_test;
uniform int al_alpha_func;
uniform float al_alpha_test_val;
uniform vec4 colorizer_color;
varying vec4 varying_color;
varying vec2 varying_texcoord;

bool alpha_test_func(float x, int op, float compare);

void main() {
  //Setup.
  float colorizer_weight = colorizer_color.a;
  float texture_weight = 1 - colorizer_weight;
  vec4 result;

  //Standard vertex tinting.
  if(al_use_tex) {
    result = varying_color * texture2D(al_tex, varying_texcoord);
  } else {
    result = varying_color;
  }
  if(al_alpha_test && !alpha_test_func(result.a, al_alpha_func, al_alpha_test_val)) {
    discard;
  }
  
  //Colorize it.
  result.r = result.r * texture_weight + colorizer_color.r * colorizer_weight;
  result.g = result.g * texture_weight + colorizer_color.g * colorizer_weight;
  result.b = result.b * texture_weight + colorizer_color.b * colorizer_weight;
  
  //Finish up.
  gl_FragColor = result;
}

bool alpha_test_func(float x, int op, float compare)
{
  if (op == 0) return false;
  else if (op == 1) return true;
  else if (op == 2) return x < compare;
  else if (op == 3) return x == compare;
  else if (op == 4) return x <= compare;
  else if (op == 5) return x > compare;
  else if (op == 6) return x != compare;
  else if (op == 7) return x >= compare;
  return false;
}

    )";

#pragma endregion
#pragma region Liquid fragment shader

//Fragment shader for sector liquids.
const char* LIQUID_FRAG_SHADER = R"(

/*
 * ========================
 * Setup
 * ========================
 */

#version 130
#ifdef GL_ES
precision mediump float;
#endif

//Fragment shader input for texture coordinates.
in vec2 varying_texcoord;

//Fragment shader input for the tint.
in vec4 varying_color;

//Fragment shader output for the final color of the fragment.
out vec4 frag_color;


/*
 * ========================
 * Uniforms
 * ========================
 */

// Time passed in the area.
uniform float area_time;

//Multiply the general distortion by this much.
uniform vec2 distortion_amount;

//Color of the water's surface.
uniform vec4 surface_color;

//Color of the caustic shines.
uniform vec4 shine_color;

//Noise values under this will have no shine [0 - 1].
uniform float shine_min_threshold;

//Noise values above this will have full shine [0 - 1].
uniform float shine_max_threshold;

//Opacity of the liquid.
uniform float opacity;

//Translation of the floor texture underneath the water.
uniform vec2 tex_translation;

//Scale of the floor texture underneath the water.
uniform vec2 tex_scale;

//Rotation of the floor texture underneath the water.
uniform float tex_rotation;

//Brightness of the sector.
uniform float sector_brightness;

//How far the sector has scrolled by.
uniform vec2 sector_scroll;

//The Allegro texture for the floor underneath the water.
uniform sampler2D al_tex;

//Dimensions of the floor texture.
uniform ivec2 bmp_size;


/*
 * ========================
 * Simplex noise functions
 * ========================
 */

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+10.0)*x);
}

vec4 taylor_inv_sqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float noise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute( permute( permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalize gradients
    vec4 norm = taylor_inv_sqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                    dot(p2,x2), dot(p3,x3) ) );
}

float color(vec2 xy, float time_scale) { return noise(vec3(1.5*xy, time_scale * area_time)); }

float simplex_noise(vec2 xy, float noise_scale, vec2 step, float time_scale) {
    float x = 0;
    x += 0.5 * color(xy * 2.0 * noise_scale - step, time_scale);
    x += 0.25 * color(xy * 4.0 * noise_scale - 2.0 * step, time_scale);
    x += 0.125 * color(xy * 8.0 * noise_scale - 4.0 * step, time_scale);
    x += 0.0625 * color(xy * 16.0 * noise_scale - 6.0 * step, time_scale);
    x += 0.03125 * color(xy * 32.0 * noise_scale - 8.0 * step, time_scale);
    return x;
}


/*
 * ========================
 * Utility functions
 * ========================
 */

//Rotates some coordinates by a given angle.
vec2 rotate(vec2 xy, float rotation) {
    vec2 result = xy;
    float c = cos(rotation);
    float s = sin(rotation);
    result.x = xy.x * c - xy.y * s;
    result.y = xy.x * s + xy.y * c;
    return result;
}

//Converts texture coordinates to world coordinates.
vec2 tex_to_world_coords(vec2 xy) {
    vec2 result = xy;
    result += tex_translation;
    result *= tex_scale;
    result = rotate(result, tex_rotation);
    return result;
}

//Converts world coordinates to texture coordinates.
vec2 world_to_tex_coords(vec2 xy) {
    vec2 result =
        vec2(
            (varying_texcoord.x + xy.x) / float(bmp_size.x),
            (-varying_texcoord.y + xy.y) / float(bmp_size.y)
        );
    return result;
}

/*
 * ========================
 * Main function
 * ========================
 */

void main() {
    //--- Basics ---

    //Define some variables that'll be used throughout the shader.
    vec2 world_coords = tex_to_world_coords(varying_texcoord);
    vec2 noise_func_step = vec2(1.3, 1.7);
    float noise_func_scale = 0.01;

    //Calculate simplex noise effects.
    float raw_noise_value = simplex_noise(world_coords - sector_scroll * area_time, noise_func_scale, noise_func_step, 0.3);

    vec2 final_noise_value = vec2(raw_noise_value, raw_noise_value);
    final_noise_value.x *= distortion_amount.x;
    final_noise_value.x *= opacity;
    final_noise_value.y *= distortion_amount.y;
    final_noise_value.y *= opacity;
    final_noise_value = rotate(final_noise_value, tex_rotation);

    //Now that we have the noise value, let's fetch the texture coords.
    vec2 target_tex_coords = world_to_tex_coords(final_noise_value);

    //Get our base texture, and tint it with the sector's tint.
    vec4 final_pixel = texture(al_tex, target_tex_coords);
    final_pixel.r *= varying_color.r;
    final_pixel.g *= varying_color.g;
    final_pixel.b *= varying_color.b;
    final_pixel.a *= varying_color.a;

    //Liquid tint.
    float liq_alpha = surface_color.a * sector_brightness;
    liq_alpha *= opacity;

    //And add it to the final output!
    final_pixel.r = final_pixel.r + (surface_color.r - final_pixel.r) * liq_alpha;
    final_pixel.g = final_pixel.g + (surface_color.g - final_pixel.g) * liq_alpha;
    final_pixel.b = final_pixel.b + (surface_color.b - final_pixel.b) * liq_alpha;


    //--- Random caustic shines ---

    //This value has a range of -1 to 1. Let's convert it to 0 to 1.
    float shine_scale = raw_noise_value;
    shine_scale += 1;
    shine_scale /= 2;

    //Scale this range to be 0 whenever shine_scale is below shine_min_threshold, 
    //and 1 whenever it is above shine_max_threshold.
    //This formula was made through too much desmos tinkering.
    shine_scale = 
        (1 / (shine_max_threshold - shine_min_threshold)) * 
        min(max(shine_min_threshold, shine_scale) - shine_min_threshold, shine_max_threshold - shine_min_threshold);

    //Anything below `shine_amount` will be below 0, resulting in it not showing.
    //This puts our scale from 0 - `shine_amount`
    shine_scale -= max(0.0, min(shine_min_threshold, 1.0));

    //Now that we're below the threshold, multiply it to return it to a [0 - 1] scale.
    //Multiply by the reciprocal to bring it back to [0 - 1].
    //Add a min value of 0.1 to prevent divide by 0 errors.
    shine_scale *= (1 / max(0.1, shine_min_threshold));

    //Do this again, but for the max threshold.
    shine_scale += max(0.0, min(shine_max_threshold, 1.0));
    shine_scale = min(1.0, shine_scale);


    //Since we haven't actually restricted negative values yet, do that now.
    shine_scale = max(shine_scale, 0.0);

    //Multiply by alpha and brightness after, since we want these to apply no matter what.
    shine_scale *= sector_brightness;
    shine_scale *= shine_color.a;
    shine_scale *= opacity;

    //Add the shine!
    final_pixel.r = final_pixel.r + (shine_color.r - final_pixel.r) * shine_scale;
    final_pixel.g = final_pixel.g + (shine_color.g - final_pixel.g) * shine_scale;
    final_pixel.b = final_pixel.b + (shine_color.b - final_pixel.b) * shine_scale;

    // TODO: remove these two lines when edge foam becomes shader-side.
    frag_color = final_pixel;
}

    )";

#pragma endregion
#pragma region Onion fragment shader
//Fragment shader for the Onion menu's background.
const char* ONION_FRAG_SHADER = R"(

/*
 * ========================
 * Setup
 * ========================
 */

#version 130
#ifdef GL_ES
precision mediump float;
#endif

//Fragment shader input for texture coordinates.
in vec2 varying_texcoord;

//Fragment shader input for the tint.
in vec4 varying_color;

//Fragment shader output for the final color of the fragment.
out vec4 frag_color;


/*
 * ========================
 * Uniforms
 * ========================
 */

// Time passed in the area.
uniform float area_time;

//Multiply the general distortion by this much.
uniform vec2 distortion_amount;

//Opacity of the liquid.
uniform float opacity;

//Texture for the colormap.
uniform sampler2D colormap;

//Brightness to use.
uniform float brightness;


/*
 * ========================
 * Simplex noise functions
 * ========================
 */

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+10.0)*x);
}

vec4 taylor_inv_sqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float noise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute( permute( permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalize gradients
    vec4 norm = taylor_inv_sqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                    dot(p2,x2), dot(p3,x3) ) );
}

float color(vec2 xy, float time_scale) { return noise(vec3(1.5*xy, time_scale * area_time)); }

float simplex_noise(vec2 xy, float noise_scale, vec2 step, float time_scale) {
    float x = 0;
    x += 0.5 * color(xy * 2.0 * noise_scale - step, time_scale);
    x += 0.25 * color(xy * 4.0 * noise_scale - 2.0 * step, time_scale);
    x += 0.125 * color(xy * 8.0 * noise_scale - 4.0 * step, time_scale);
    x += 0.0625 * color(xy * 16.0 * noise_scale - 6.0 * step, time_scale);
    x += 0.03125 * color(xy * 32.0 * noise_scale - 8.0 * step, time_scale);
    return x;
}


/*
 * ========================
 * Main function
 * ========================
 */

void main() {
    //--- Basics ---

    //Define some variables that'll be used throughout the shader.
    vec2 noise_func_step = vec2(1.3, 1.7);
    float noise_func_scale = 0.1;

    //Calculate simplex noise effects.
    float raw_noise_value = simplex_noise(varying_texcoord, noise_func_scale, noise_func_step, 0.02);
    raw_noise_value = simplex_noise(varying_texcoord + raw_noise_value, noise_func_scale, noise_func_step, 0.02);
    vec4 final_pixel = texture(colormap, vec2((raw_noise_value + 0.2) * 2.5, 0));
    final_pixel.a = opacity;
    final_pixel.r *= brightness;
    final_pixel.g *= brightness;
    final_pixel.b *= brightness;
    frag_color = final_pixel;

}

    )";
}
