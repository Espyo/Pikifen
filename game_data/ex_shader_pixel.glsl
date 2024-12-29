#ifdef GL_ES
precision mediump float;
#endif

#define MAX_FOAM_EDGES 256

uniform sampler2D al_tex;
uniform float time;

//Formatted in (x1, y1, x2, y2)
uniform vec4 foamEdges[MAX_FOAM_EDGES];
uniform int edge_count;

uniform vec2 tex_size;
uniform vec4 liq_tint;
//Has the textures tint already applied.
varying vec4 varying_color;

//
uniform float fill_level;

uniform vec2 tex_translation;
uniform vec2 tex_scale;
uniform float tex_rotation;

//World Position
varying vec2 varying_texcoord;

//
// GLSL textureless classic 3D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2024-11-07
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x)
{
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
   return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
   return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
   return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec3 P)
{
   vec3 Pi0 = floor(P); // Integer part for indexing
   vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
   Pi0 = mod289(Pi0);
   Pi1 = mod289(Pi1);
   vec3 Pf0 = fract(P); // Fractional part for interpolation
   vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
   vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
   vec4 iy = vec4(Pi0.yy, Pi1.yy);
   vec4 iz0 = Pi0.zzzz;
   vec4 iz1 = Pi1.zzzz;

   vec4 ixy = permute(permute(ix) + iy);
   vec4 ixy0 = permute(ixy + iz0);
   vec4 ixy1 = permute(ixy + iz1);

   vec4 gx0 = ixy0 * (1.0 / 7.0);
   vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
   gx0 = fract(gx0);
   vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
   vec4 sz0 = step(gz0, vec4(0.0));
   gx0 -= sz0 * (step(0.0, gx0) - 0.5);
   gy0 -= sz0 * (step(0.0, gy0) - 0.5);

   vec4 gx1 = ixy1 * (1.0 / 7.0);
   vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
   gx1 = fract(gx1);
   vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
   vec4 sz1 = step(gz1, vec4(0.0));
   gx1 -= sz1 * (step(0.0, gx1) - 0.5);
   gy1 -= sz1 * (step(0.0, gy1) - 0.5);

   vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
   vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
   vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
   vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
   vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
   vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
   vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
   vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

   vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
   vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));

   float n000 = norm0.x * dot(g000, Pf0);
   float n010 = norm0.y * dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
   float n100 = norm0.z * dot(g100, vec3(Pf1.x, Pf0.yz));
   float n110 = norm0.w * dot(g110, vec3(Pf1.xy, Pf0.z));
   float n001 = norm1.x * dot(g001, vec3(Pf0.xy, Pf1.z));
   float n011 = norm1.y * dot(g011, vec3(Pf0.x, Pf1.yz));
   float n101 = norm1.z * dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
   float n111 = norm1.w * dot(g111, Pf1);

   vec3 fade_xyz = fade(Pf0);
   vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
   vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
   float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
   return 2.2 * n_xyz;
}

// demo code:
float color(vec2 xy) { return cnoise(vec3(1.5*xy, 0.3*time)); }

float perlin_noise(vec2 xy, float noiseScale, vec2 step) {
   float x = 0;
   x += 0.5 * color(xy * 2.0 * noiseScale - step);
   x += 0.25 * color(xy * 4.0 * noiseScale - 2.0 * step);
   x += 0.125 * color(xy * 8.0 * noiseScale - 4.0 * step);
   x += 0.0625 * color(xy * 16.0 * noiseScale - 6.0 * step);
   x += 0.03125 * color(xy * 32.0 * noiseScale - 8.0 * step);
   return x;
}
vec2 rotate(vec2 xy, float rotation) {
   vec2 output = xy;
   float c = cos(rotation);
   float s = sin(rotation);
   output.x = xy.x * c - xy.y * s;
   output.y = xy.x * s + xy.y * c;
   return output;
}
vec2 toWorldCoords(vec2 xy)
{

   vec2 output = xy;
   output += tex_translation;

   output *= tex_scale;

   output = rotate(output, tex_rotation);

   return output;
}

float getDistFromEdge(vec2 xy) {

   //Arbitrarily large number my beloved.
   float minDist = 100000;
   for(int i = 0; i < MAX_FOAM_EDGES; i++) {
      if(i >= edge_count) return minDist;
      vec2 v1 = foamEdges[i].xy;
      vec2 v2 = foamEdges[i].zw;

      //code from http://stackoverflow.com/a/3122532
      vec2 v1_to_p = xy - v1;
      vec2 v1_to_v2 = v2 - v1;

      float v1_to_v2_squared = (v1_to_v2.x * v1_to_v2.x) + (v1_to_v2.y * v1_to_v2.y);

      float v1_to_p_dot_v1_to_v2 = (v1_to_p.x * v1_to_v2.x) + (v1_to_p.y * v1_to_v2.y);

      float r = max(0, min(1, v1_to_p_dot_v1_to_v2 / v1_to_v2_squared));

      vec2 closest_point = v1 + (v1_to_v2 * r);

      vec2 p_to_c = closest_point - xy;

      float dist = sqrt(pow(p_to_c.x, 2) + pow(p_to_c.y, 2));

      //Add a lowest possible value to prevent seams
      minDist = max(1, min(minDist, dist));
   }
   return minDist;
}

void main()
{
   vec2 worldCoords = toWorldCoords(varying_texcoord);

   vec2 step = vec2(1.3, 1.7);
   float noiseScale = 0.01;

   float nX = perlin_noise(worldCoords, noiseScale, step);
   float effectScaleX = 14;
   nX *= effectScaleX;
   nX *= fill_level;

   float nY = perlin_noise(worldCoords, noiseScale, step);
   float effectScaleY = 4;
   nY *= effectScaleY;
   nY *= fill_level;

   vec2 pEffect = rotate(vec2(nX, nY), tex_rotation);
   //Convert from world coords to texture coords
   vec2 sample_texcoord =
      vec2((varying_texcoord.x + pEffect.x) / tex_size.x, (-varying_texcoord.y + pEffect.y) / tex_size.y);

   vec4 tmp = texture2D(al_tex, sample_texcoord);

   //Base tint
   tmp.r *= varying_color.r;
   tmp.g *= varying_color.g;
   tmp.b *= varying_color.b;
   tmp.a *= varying_color.a;

   //Liquid tint
   float liq_alpha = liq_tint.a + (nX / (effectScaleX * 7)) * 0.5;
   liq_alpha *= fill_level;

   tmp.r *= 1 + (liq_tint.r - 1) * liq_alpha;
   tmp.g *= 1 + (liq_tint.g - 1) * liq_alpha;
   tmp.b *= 1 + (liq_tint.b - 1) * liq_alpha;

   //Random shines
   float shineScale = max(0, (nX / effectScaleX) - 0.4);

   tmp.r += shineScale * 2;
   tmp.g += shineScale * 2;
   tmp.b += shineScale * 2;

   //Edge foam
   float edgeSize = 11 + (10 + sin(time) * 5) * perlin_noise(worldCoords, 0.04, step);
   edgeSize *= fill_level;
   float edgeScale = min(0.6, max((edgeSize - getDistFromEdge(worldCoords)) / edgeSize - 0.1 , 0));

   tmp.r += edgeScale;
   tmp.g += edgeScale;
   tmp.b += edgeScale;

   gl_FragColor = tmp;
}
