#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 C;

//
void main()
{
    C = a_uv;
	gl_Position = vec4(a_position, 0.0, 1.0);

}


#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 out_color;

in vec2 C;

//
void main()
{
    out_color = vec4(C, 0.0, 1.0);

}


