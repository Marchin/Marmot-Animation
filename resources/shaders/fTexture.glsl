#version 330 core

in vec2 vTexCoords;

out vec4 fColor;

uniform sampler2D tex;

void main() {  
    if (vTexCoords.x < 0.0f  || vTexCoords.y < 0.0f) {
        discard;
    }  
   
    fColor = texture(tex, vTexCoords);
}