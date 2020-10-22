#version 430

// Set the number of invocations in the work group.
// In this case, we operate on the image in 16x16 pixel tiles.
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, r32f) uniform highp readonly image2D warpedBrushTexture;
layout(binding = 1, rgba32f) uniform highp image2D tex_K;
layout(binding = 2, rgba32f) uniform highp image2D tex_S;
layout(binding = 3, rgba32f) uniform highp image2D tex_smudge_K;
layout(binding = 4, rgba32f) uniform highp image2D tex_smudge_S;

uniform ivec2 offset;

uniform highp vec2 rotationCenter;
uniform highp float theta;
uniform ivec2 topLeft;
uniform highp float thicknessScale;

const highp float rateDeposition = 0.1;
const highp float ratePickup     = 0.1;

highp vec2 rotate(in highp vec2 p, in highp vec2 center, in highp float theta) {
  highp vec2 pt = p - center;
  highp vec2 pr = vec2(pt.x * cos(theta) - pt.y * sin(theta),
                       pt.x * sin(theta) + pt.y * cos(theta));
  return pr + center;
}

void main() {
  ivec2 gSize = imageSize(tex_K);

  // flip y and add offset of bounding box around the warped brush footprint
  ivec2 canvasPos = ivec2(offset.x + gl_GlobalInvocationID.x,
                          gSize.y - (offset.y + gl_GlobalInvocationID.y) - 1);

  // don't access outliers
  if ((canvasPos.x < 0) || (canvasPos.y < 0) || (canvasPos.x >= gSize.x) ||
      (canvasPos.y >= gSize.y)) {
    return;
  }

  highp float vTex =
    thicknessScale * imageLoad(warpedBrushTexture, canvasPos).r;

  if (vTex < 10e-3) {
    return;
  }

  // get the position of the smudge map
  ivec2 smudgePos = canvasPos - topLeft;

  // rotate the position of the smudge map
  highp vec2 rotatedSmudgePosF =
    rotate(vec2(smudgePos.x, smudgePos.y), rotationCenter, theta);
  ivec2 rotatedSmudgePos = ivec2(rotatedSmudgePosF.x, rotatedSmudgePosF.y);

  // don't access outliers
  ivec2 pSize = imageSize(tex_K);
  if ((rotatedSmudgePos.x < 0) || (rotatedSmudgePos.y < 0) ||
      (rotatedSmudgePos.x >= pSize.x) || (rotatedSmudgePos.y >= pSize.y)) {
    return;
  }

  // load the current state
  highp vec3 canvasK = imageLoad(tex_K, canvasPos).rgb;
  highp vec3 canvasS = imageLoad(tex_S, canvasPos).rgb;
  highp float cV     = imageLoad(tex_K, canvasPos).a;

  highp vec3 pickK = imageLoad(tex_smudge_K, rotatedSmudgePos).rgb;
  highp vec3 pickS = imageLoad(tex_smudge_S, rotatedSmudgePos).rgb;
  highp float pV   = imageLoad(tex_smudge_K, rotatedSmudgePos).a;

  // volume of paint pickup from canvas
  highp float cVl = cV * rateDeposition * vTex;
  highp float cVr = cV - cVl;

  // volume of paint distributed to canvas
  highp float pVl = pV * ratePickup * vTex;
  highp float pVr = pV - pVl;

  // pickup from canvas
  highp float pVnew = pVr + cVl;
  if (pVnew > 0.0) {
    imageStore(tex_smudge_K, rotatedSmudgePos,
               vec4((pVr * pickK + cVl * canvasK) / pVnew, max(pVnew, 0.0)));
    imageStore(tex_smudge_S, rotatedSmudgePos,
               vec4((pVr * pickS + cVl * canvasS) / pVnew, max(pVnew, 0.0)));
  }

  // deposition to canvas
  highp float cVnew = cVr + pVl;
  if (cVnew > 0.0) {
    imageStore(tex_K, canvasPos,
               vec4((cVr * pickK + pVl * canvasK) / cVnew, max(cVnew, 0.0)));
    imageStore(tex_S, canvasPos,
               vec4((cVr * pickS + pVl * canvasS) / cVnew, vTex));
  }
}
