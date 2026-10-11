#pragma once

// Scenes render into a linear buffer that RenderGLES2 encodes to sRGB at the end.
// Colors are authored sRGB-encoded and decoded with these.

#include <cmath>

// sRGB-encoded [0,1] to linear
inline float srgbToLinear(float c)
{
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

// GLSL counterpart, spliced into fragment shaders after the precision statement.
// Clamps first, as the RGBA8 targets the scenes were written for did.
#define SRGB_TO_LINEAR_GLSL R"(
  highp vec3 srgbToLinear(highp vec3 c)
  {
      c = clamp(c, 0.0, 1.0);
      return mix(pow((c + 0.055) / 1.055, vec3(2.4)), c / 12.92, vec3(lessThanEqual(c, vec3(0.04045))));
  }
)"
