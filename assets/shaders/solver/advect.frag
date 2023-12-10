#type FRAGMENT_SHADER
#version 450 core

layout (location=0) out vec4 Q;

in vec2 C;

uniform float u_dt;
uniform vec2 u_world2tx;

uniform sampler2D u_velocity;
uniform sampler2D u_quantity;

//
vec2 v(vec2 tx)
{
    vec2 vel = texture(u_velocity, tx).xy;

    if(tx.x < 0.0 || tx.x > 1.0) vel.x = -vel.x;
    if(tx.y < 0.0 || tx.y > 1.0) vel.y = -vel.y;

    return vel;

}

//
vec2 RK4()
{
    vec2 k1 = v(C);
    vec2 k2 = v(C - u_world2tx * 0.5 * k1 * u_dt);
    vec2 k3 = v(C - u_world2tx * 0.5 * k2 * u_dt);
    vec2 k4 = v(C - u_world2tx * k3 * u_dt);
    return C - u_world2tx * u_dt * (k1 + 2.0 * (k2 + k3) + k4) / 6.0;

}

//
void main()
{
    // backward integrate to previous position using Runge-Kutta 4
    Q = texture(u_quantity, RK4());

}