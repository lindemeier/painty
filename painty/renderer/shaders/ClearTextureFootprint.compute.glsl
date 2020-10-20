#version 430

// Set the number of invocations in the work group.
// In this case, we operate on the image in 16x16 pixel tiles.
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, r32f) uniform highp writeonly image2D warpedBrushTexture;

uniform ivec2 offset;

void main() {
  ivec2 gSize = imageSize(warpedBrushTexture);

  // flip y and add offset of bounding box around the warped brush footprint
  ivec2 texPos = ivec2(offset.x + gl_GlobalInvocationID.x,
                       gSize.y - (offset.y + gl_GlobalInvocationID.y) - 1);

  // don't access outliers
  if (texPos.x < 0 || texPos.y < 0 || texPos.x >= gSize.x ||
      texPos.y >= gSize.y) {
    return;
  }

  imageStore(warpedBrushTexture, texPos, vec4(0.0, 0.0, 0.0, 0.0));
}
