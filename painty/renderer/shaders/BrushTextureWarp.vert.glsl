
#version 330 core

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;

uniform mat4 projectionMatrix;

out vec2 texCoord;

void main() {
  texCoord = vertexTexCoord;

  gl_Position = projectionMatrix * vec4(vertexPosition.xy, 0.0, 1.0);
}
