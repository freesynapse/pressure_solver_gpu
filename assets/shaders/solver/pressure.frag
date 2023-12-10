#type FRAGMENT_SHADER
#version 450 core

layout (location = 0) out vec4 P;

layout (binding = 1) uniform sampler2D u_pressure;
layout (binding = 0) uniform sampler2D u_divergence;

uniform float u_dx2;

in vec2 C, L, R, T, B;

#define p(tx) texture(u_pressure, tx).x

//
void main()
{
    float div = texture(u_divergence, C).x;
    P = vec4((p(L) + p(R) + p(T) + p(B) - u_dx2 * div) / 4.0, 0.0, 0.0, 1.0);

}
