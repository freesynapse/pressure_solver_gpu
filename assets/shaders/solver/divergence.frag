#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 divergence;

layout(binding = 0) uniform sampler2D u_velocity;

uniform float u_half_inv_dx;

in vec2 L, R, T, B;

//
vec2 v(vec2 tx)
{
    vec2 vel = texture(u_velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;

}

//
void main()
{
    float div = u_half_inv_dx * (v(R).x - v(L).x + v(T).y - v(B).y);
    divergence = vec4(div, 0.0, 0.0, 1.0);

}
