#version 330 core
in vec3 vert_color;  
in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec4 prog_color;

void main() {
    frag_color = mix(texture(tex, tex_coord) * prog_color, texture(tex2, tex_coord * vec2(2.0, 2.0)) * prog_color * vec4(vert_color, 1.0), 0.5); 
}
