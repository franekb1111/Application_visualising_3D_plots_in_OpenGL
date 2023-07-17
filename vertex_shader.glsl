#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 MVP;
uniform float min_z;
uniform float max_z;

out float nHight;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0); //pozycja wierzcholkow
    nHight = (vPos.z - min_z) / (max_z - min_z); //znormalizowana wyskosc
}