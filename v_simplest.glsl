#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec4 vertex;
in vec2 texcoord;
in vec3 normal;
out vec2 vTexcoord;
out vec3 vNormal;
out vec3 vFragPos;

void main(void) {
    gl_Position = P * V * M * vertex;
    vTexcoord = texcoord;
    vFragPos = vec3(M * vertex); // World space position
    vNormal = mat3(transpose(inverse(M))) * normal;
}
