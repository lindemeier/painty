#version 430

// Set the number of invocations in the work group.
// In this case, we operate on the image in 16x16 pixel tiles.
layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform highp image2D tex_R0;
layout(binding = 1, rgba32f) uniform highp readonly image2D tex_K;
layout(binding = 2, rgba32f) uniform highp readonly image2D tex_S;
layout(binding = 3, r32f) uniform highp readonly image2D tex_V;

uniform ivec2 offset;

highp float coth(in highp float x) {
  if (x > 20.0) {
    return 1.0;
  }
  if (abs(x) > 0.0) {
    return cosh(x) / sinh(x);
  } else {
    return 1.0 / 0.0;
  }
}

highp vec3 coth(in highp vec3 v) {
  return vec3(coth(v.x), coth(v.y), coth(v.z));
}

void main() {
  ivec2 gSize = imageSize(tex_R0);
  if (gl_GlobalInvocationID.x < 0 || gl_GlobalInvocationID.y < 0 ||
      gl_GlobalInvocationID.x >= gSize.x ||
      gl_GlobalInvocationID.y >= gSize.y) {
    return;
  }

  ivec2 texPos = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);

  highp vec3 r0 = imageLoad(tex_R0, texPos).rgb;
  highp vec3 K  = imageLoad(tex_K, texPos).rgb;
  highp vec3 S  = imageLoad(tex_S, texPos).rgb;
  highp float d = imageLoad(tex_V, texPos).r;

  const float Eps = 10e-6;
  if ((abs(d) < Eps) || (abs(S.r) < Eps) || (abs(S.g) < Eps) ||
      (abs(S.b) < Eps)) {
    return;
  }
  highp vec3 K_S = K / S;
  highp vec3 a   = vec3(1.0, 1.0, 1.0) + K_S;

  highp vec3 asq = a * a;

  highp vec3 b = sqrt(max(asq - vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0)));

  highp vec3 bSh = b * S * d;

  highp vec3 bcothbSh = coth(bSh);

  highp vec3 R =
    (vec3(1.0, 1.0, 1.0) - r0 * (a - bcothbSh)) / (a - r0 + bcothbSh);

  imageStore(tex_R0, texPos, vec4(R, 1.0));
}
