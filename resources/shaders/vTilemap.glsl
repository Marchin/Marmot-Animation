#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in int aID;

out vec2 vTexCoords;

uniform mat4 uModelViewProjection;
uniform float uTileSize;
uniform int uMapWidth;
uniform int uVisibleWidth;
uniform int uCamX;
uniform int uCamY;

void main() {
    vTexCoords = aTexCoords;
    vec2 pos = aPos + vec2(gl_InstanceID%uMapWidth, gl_InstanceID/uMapWidth);
    gl_Position = uModelViewProjection * vec4(pos, 0.0, 1.0);
}