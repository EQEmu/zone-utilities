#version 330 core

layout(location = 0) in vec3 vp_ms;
layout(location = 1) in vec3 vert_color;

uniform mat4 MVP;
uniform vec3 Tint;
out vec3 frag_color;

void main(){
	gl_Position.x = vp_ms.x;
	gl_Position.y = vp_ms.z;
	gl_Position.z = vp_ms.y;
	gl_Position.w = 1.0;
    gl_Position = MVP * gl_Position;
	frag_color = vert_color * Tint;
}

