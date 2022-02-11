#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv; 
out vec3 vert_color;
out vec2 tex_coord;
uniform mat4 trs;

void main() {
    // gl_Position is a static key
    gl_Position = trs * vec4(pos, 1.0);
    vert_color = color;
    tex_coord = uv;
}
