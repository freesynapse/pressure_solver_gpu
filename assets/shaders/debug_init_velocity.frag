#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 color;

in vec2 C;

vec2 f(vec2 C)
{
    vec2 c = C - 0.5;
    
    return -2 * c;
    // return normalize(-2*c);
    // return normalize(vec2(-0.5, 0.5));
}

void main()
{
    color = vec4(f(C), 0.0, 1.0);

}
