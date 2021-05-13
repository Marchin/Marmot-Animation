#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 vPos;

uniform mat4 uModelViewProjection;

void main(){
	vPos = aPos;
	gl_Position =  uModelViewProjection * vec4(aPos, 1.0f);
}