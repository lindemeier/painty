#version 420 core

layout(binding = 0) uniform sampler2D brushTexture;

in vec2 texCoord;

out vec3 color;

void main() {
  color = texture(brushTexture, texCoord).rgb;
  // color = vec3(1.0,1.0, 1.0);
}
