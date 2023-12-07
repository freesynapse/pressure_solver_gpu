#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec2 a_position;
layout(location = 4) in vec2 a_uv;

uniform float u_texel_size;

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

uniform sampler2D u_sampler_velocity;

//
void main()
{
    // float c = texture(u_sampler, v_uv);
	// out_color = vec4(c, 0.0, 0.0, 1.0);
    out_color = vec4(C, 0.0, 1.0);
}


