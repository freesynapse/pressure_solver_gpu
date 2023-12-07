#type VERTEX_SHADER
#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 v_uv;

//
void main()
{
	v_uv = a_uv;
	gl_Position = vec4(a_position, 1.0);
}


#type FRAGMENT_SHADER
#version 450 core

layout(location = 0) out vec4 out_color;

uniform sampler2D u_text_sampler;

in vec2 v_uv;

//
void main()
{
	float text_color = texture(u_text_sampler, v_uv).r;
    out_color = vec4(text_color, 0.0, 0.0, 1.0);

}


