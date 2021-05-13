#version 330 core

in vec2 vTexCoords;

out vec4 fColor;

uniform sampler2D texture_diffuse1;

void main() {  
    fColor = texture(texture_diffuse1, vTexCoords);
}