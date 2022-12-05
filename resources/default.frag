#version 460

out vec4 frag_color;
in vec3 normal;
in vec3 frag_pos;

uniform vec3 color;
uniform vec3 cam_pos;

void main() {
    frag_color = vec4(color, 1.f);
}
