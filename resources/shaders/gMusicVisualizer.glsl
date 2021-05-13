#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

in VS_OUT {
    vec4 color;
} gs_in[];

out vec4 gColor;

uniform mat4 viewProj;
uniform float scale;

void build_cube(vec4 position) {
    mat4 model = mat4(0);
    model[0][0] = scale;
    model[1][1] = scale;
    model[2][2] = scale;
    model[3] = position;
    
    mat4 mvp = viewProj;
    
    gl_Position = mvp * model * vec4(-.5f, .5f, .5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(.5f, .5f, .5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(-.5f, -.5f, .5f, 1.f);
    EmitVertex();
    
    gl_Position = mvp * model * vec4(.5f, -.5f, .5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(.5f, -.5f, -.5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(.5f, .5f, .5f, 1.f);
    EmitVertex();
    
    gl_Position = mvp * model * vec4(.5f, .5f, -.5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(-.5f, .5f, .5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(-.5f, .5f, -.5f, 1.f);
    EmitVertex();
    
    gl_Position = mvp * model * vec4(-.5f, -.5f, .5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(-.5f, -.5f, -.5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(.5f, -.5f, -.5f, 1.f);
    EmitVertex();
    
    gl_Position = mvp * model * vec4(-.5f, .5f, -.5f, 1.f);
    EmitVertex();
    gl_Position = mvp * model * vec4(.5f, .5f, -.5f, 1.f);
    EmitVertex();
    
    EndPrimitive();
}

void main() {
    gColor = gs_in[0].color;
    build_cube(gl_in[0].gl_Position);
}