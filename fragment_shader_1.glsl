#version 330 core

out vec4 FragColor;
in float nHight;

void main() {
    FragColor = vec4(nHight, 1 - nHight, 0.0, 1.0); // kolor wypelnienia siatki trojkatow
}
