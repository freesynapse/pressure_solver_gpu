#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out float divergence;

layout(binding = 0) uniform sampler2D u_velocity_sampler;
uniform float u_half_inv_dx;

in vec2 L, R, T, B;

//
vec2 v(vec2 tx)
{
    vec2 vel = texture(u_velocity_sampler, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;

}

//
void main()
{
    divergence = u_half_inv_dx * (v(R).x - v(L).x + v(T).y - v(B).y);

}
