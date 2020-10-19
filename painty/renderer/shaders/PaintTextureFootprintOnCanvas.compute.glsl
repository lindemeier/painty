#version 430

// Set the number of invocations in the work group.
// In this case, we operate on the image in 16x16 pixel tiles.
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, r32f) uniform highp readonly image2D warpedBrushTexture;
layout(binding = 1, rgba32f) uniform highp image2D tex_K;
layout(binding = 2, rgba32f) uniform highp image2D tex_S;

uniform ivec2 offset;

uniform highp vec3 K_brush;
uniform highp vec3 S_brush;

void main() {
  ivec2 gSize = imageSize(tex_K);

  // flip y and add offset of bounding box around the warped brush footprint
  ivec2 texPos = ivec2(offset.x + gl_GlobalInvocationID.x,
                        gSize.y - (offset.y + gl_GlobalInvocationID.y) - 1);

  // don't access outliers
  if (texPos.x < 0 || texPos.y < 0 || texPos.x >= gSize.x ||
      texPos.y >= gSize.y) {
    return;
  }

  // load the current state
  highp vec3 K     = imageLoad(tex_K, texPos).rgb;
  highp vec3 S     = imageLoad(tex_S, texPos).rgb;
  highp float vCan = imageLoad(tex_K, texPos).a;
  highp float vTex = imageLoad(warpedBrushTexture, texPos).r;

  // compute the total amount of paint
  highp float vSum = vCan + vTex;

  // only if enough paint will be distributed
  const float Eps = 10e-6;
  if (vSum < Eps) {
    return;
  }

  // combine the paints weighted by volume
  highp vec3 Kn = (vCan * K + vTex * K_brush) / vSum;
  highp vec3 Sn = (vCan * S + vTex * S_brush) / vSum;

  // store the results
  imageStore(tex_K, texPos, vec4(Kn, vSum));
  imageStore(tex_S, texPos, vec4(Sn, vSum));
}
