#version 130

in vec2 uv;

out vec4 color;

uniform sampler2D tex_sample;

void main()
{
	color = texture2D(tex_sample, uv).rgba;
}
