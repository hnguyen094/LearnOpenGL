#version 330 core
in vec3 vert_color;  
in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D tex;
uniform sampler2D tex2;

void main() {
    frag_color = mix(texture(tex, tex_coord), texture(tex2, tex_coord * vec2(2.0, 2.0)) * vec4(vert_color, 1.0), 0.5); 
    // frag_color = texture(tex, tex_coord);
}
