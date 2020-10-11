#version 430

// Set the number of invocations in the work group.
// In this case, we operate on the image in 16x16 pixel tiles.
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgb32f) uniform highp image2D tex_R0;
layout(binding = 1, rgb32f) uniform highp readonly image2D tex_K;
layout(binding = 2, rgb32f) uniform highp readonly image2D tex_S;
layout(binding = 3, r32f) uniform highp readonly image2D tex_V;

uniform ivec2 offset;
uniform ivec2 gSize;

void main() {
  if (gl_GlobalInvocationID.x < 0 || gl_GlobalInvocationID.y < 0 ||
      gl_GlobalInvocationID.x >= gSize.x ||
      gl_GlobalInvocationID.y >= gSize.y) {
    return;
  }

  ivec2 texPos = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);

  highp vec4 r0 = imageLoad(tex_R0, texPos);

  imageStore(tex_R0, texPos, vec4(1.0, 0.0, 0.0, 1.0));
}
