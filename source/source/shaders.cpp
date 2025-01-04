/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Shader related functions.
 */

#include "shaders.h"
#include "functions.h"
#include "game.h"

shader_manager::shader_manager(){}


ALLEGRO_SHADER* shader_manager::get_shader(SHADER_TYPE shader_type) {
    assert(shader_type != N_SHADER_TYPES);

    return compiled_shaders[(int)shader_type];
}


void shader_manager::compile_shaders() {

//const char* def_vert_shader
#pragma region Default Vertex Shader
const char* def_vert_shader = R"(
#version 430
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

//const char* liquid_pixel_shader
#pragma region Liquid Fragment Shader
char* liquid_pixel_shader = R"(
#version 430
#extension GL_ARB_shader_storage_buffer_object: enable

#ifdef GL_ES
precision mediump float;
#endif

#define MAX_FOAM_EDGES 65535 * 4

uniform sampler2D al_tex;
uniform float time;

readonly layout(std430, binding = 3) buffer foamLayout
{
   //Formatted in (x1, y1, x2, y2)
   readonly float foamEdges[];
};

uniform int edge_count;

uniform vec2 effect_scale;
uniform vec4 liq_tint;

uniform vec4 shine_tint;
uniform float shine_threshold;

uniform vec4 foam_tint;
uniform float foam_size;

in vec4 varying_color;

uniform float fill_level;

uniform ivec2 tex_size;
uniform float tex_brightness;
uniform vec2 tex_translation;
uniform vec2 tex_scale;
uniform float tex_rotation;

//World Position
in vec2 varying_texcoord;

out vec4 fragColor;

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

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float noise(vec3 v)
  { 
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

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
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

float color(vec2 xy, float timeScale) { return noise(vec3(1.5*xy, timeScale*time)); }

float simplex_noise(vec2 xy, float noiseScale, vec2 step, float timeScale) {
   float x = 0;
   x += 0.5 * color(xy * 2.0 * noiseScale - step, timeScale);
   x += 0.25 * color(xy * 4.0 * noiseScale - 2.0 * step, timeScale);
   x += 0.125 * color(xy * 8.0 * noiseScale - 4.0 * step, timeScale);
   x += 0.0625 * color(xy * 16.0 * noiseScale - 6.0 * step, timeScale);
   x += 0.03125 * color(xy * 32.0 * noiseScale - 8.0 * step, timeScale);
   return x;
}
vec2 rotate(vec2 xy, float rotation) {
   vec2 product = xy;
   float c = cos(rotation);
   float s = sin(rotation);
   product.x = xy.x * c - xy.y * s;
   product.y = xy.x * s + xy.y * c;
   return product;
}
vec2 toWorldCoords(vec2 xy)
{
   vec2 product = xy;
   product += tex_translation;

   product *= tex_scale;

   product = rotate(product, tex_rotation);

   return product;
}

float getDistFromEdge(vec2 xy) {

   //Arbitrarily large number my beloved.
   float minDist = 100000;
   for(int i = 0; i < MAX_FOAM_EDGES; i++) {
      if(i >= edge_count) return minDist;
      vec2 v1 = vec2(foamEdges[4 * i], foamEdges[(4 * i) + 1]);
      vec2 v2 = vec2(foamEdges[(4 * i) + 2], foamEdges[(4 * i) + 3]);

      //code from http://stackoverflow.com/a/3122532
      vec2 v1_to_p = xy - v1;
      vec2 v1_to_v2 = v2 - v1;

      float v1_to_v2_squared = (v1_to_v2.x * v1_to_v2.x) + (v1_to_v2.y * v1_to_v2.y);

      float v1_to_p_dot_v1_to_v2 = (v1_to_p.x * v1_to_v2.x) + (v1_to_p.y * v1_to_v2.y);

      float r = max(0.0, min(1.0, v1_to_p_dot_v1_to_v2 / v1_to_v2_squared));

      vec2 closest_point = v1 + (v1_to_v2 * r);

      vec2 p_to_c = closest_point - xy;

      float dist = sqrt(pow(p_to_c.x, 2.0) + pow(p_to_c.y, 2.0));

      //Add a lowest possible value to prevent seams
      minDist = max(1.0, min(minDist, dist));
   }
   return minDist;
}

void main()
{
   //Define some variables that'll be used throughout the shader
   vec2 worldCoords = toWorldCoords(varying_texcoord);
   vec2 step = vec2(1.3, 1.7);
   float noiseScale = 0.01;
   
   //Calculate simplex noise effects.
   float nX = simplex_noise(worldCoords, noiseScale, step, 0.3);
   nX *= effect_scale.x;
   nX *= fill_level;
   float nY = simplex_noise(worldCoords, noiseScale, step, 0.3);
   nY *= effect_scale.y;
   nY *= fill_level;
   vec2 pEffect = rotate(vec2(nX, nY), tex_rotation);
   
   //Convert from world coords to texture coords with the simplex noise effect applied.
   vec2 sample_texcoord =
      vec2(
         (varying_texcoord.x + pEffect.x) / float(tex_size.x), 
         (-varying_texcoord.y + pEffect.y) / float(tex_size.y)
      );

   //Get our base texture, and tint it to the sector's tint.
   vec4 tmp = texture2D(al_tex, sample_texcoord);
   tmp.r *= varying_color.r;
   tmp.g *= varying_color.g;
   tmp.b *= varying_color.b;
   tmp.a *= varying_color.a;

   //Liquid tint
   float liq_alpha = liq_tint.a * tex_brightness;
   liq_alpha *= fill_level;

   //And add it to the final output!
   tmp.r = tmp.r + (liq_tint.r - tmp.r) * liq_alpha;
   tmp.g = tmp.g + (liq_tint.g - tmp.g) * liq_alpha;
   tmp.b = tmp.b + (liq_tint.b - tmp.b) * liq_alpha;



   // -- Random shines --
   //Get the average of each effect.
   float shineScale = (nX / effect_scale.x) + (nY / effect_scale.y);
   shineScale /= 2;

   //Anything below `shine_threshold` will be below 0, resulting in it not showing.
   //This puts our scale from 0 - (1 - `shine_threshold`)
   shineScale -= (1 - shine_threshold);

   //Now that we're below the threshold, multiply it to return it to a 0-1 scale.
   //Multiply by the reciprocal to bring it back to 0 - 1;
   //Add a min value of 0.1 to prevent divide by 0 errors.
   shineScale *= (1 / max(0.1, 1 - shine_threshold));

   //Since we havent actually restricted negative values yet, do that now.
   shineScale = max(shineScale, 0.0);

   //Multiply by alpha and brightness after, since we want these to apply no matter what.
   shineScale *= tex_brightness;
   shineScale *= shine_tint.a;

   //Add the shine!
   tmp.r = tmp.r + (shine_tint.r - tmp.r) * shineScale;
   tmp.g = tmp.g + (shine_tint.g - tmp.g) * shineScale;
   tmp.b = tmp.b + (shine_tint.b - tmp.b) * shineScale;



   // -- Edge Foam -- 
   float maxDist = foam_size;

   //Apply some effects to make sure the foam isnt a straight line.
   //These parameters aren't super intuitive, so they aren't exposed to the liquid proper.
   float effectScale = maxDist / 25;

   //Next, add a simplex noise effect to introduce an uneven fade.
   maxDist += ((2 * simplex_noise(worldCoords, 0.02, step, 0.2)) - 1) * 7 * effectScale;

   //Prevent foam from entirely disappearing...
   maxDist = max(1.0, maxDist);

   //...unless we're draining the liquid.
   maxDist *= fill_level;

   //Now using this distance, get a 0-1 number for how far away it is.
   float edgeScale = maxDist - getDistFromEdge(worldCoords);
   edgeScale /= maxDist;

   //This ensures there's no negatives, and also adds a small flattening as it approaches 1
   edgeScale = clamp(edgeScale, 0.0, 1.0);

   //Add the alpha and brightness.
   edgeScale *= foam_tint.a;
   edgeScale *= tex_brightness;

   //And add it to the full thing!
   tmp.r = tmp.r + (foam_tint.r - tmp.r) * edgeScale;
   tmp.g = tmp.g + (foam_tint.g - tmp.g) * edgeScale;
   tmp.b = tmp.b + (foam_tint.b - tmp.b) * edgeScale;

   fragColor = tmp;
}
)";
#pragma endregion


//Create the shader
compiled_shaders[SHADER_TYPE_LIQUID] = al_create_shader(ALLEGRO_SHADER_GLSL);
try_attach_shader(compiled_shaders[SHADER_TYPE_LIQUID], ALLEGRO_PIXEL_SHADER, liquid_pixel_shader);
try_attach_shader(compiled_shaders[SHADER_TYPE_LIQUID], ALLEGRO_VERTEX_SHADER, def_vert_shader);
al_build_shader(compiled_shaders[SHADER_TYPE_LIQUID]);

}


void shader_manager::try_attach_shader(ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* source) {
   engine_assert(al_attach_shader_source(shader, type, source), al_get_shader_log(shader));
}