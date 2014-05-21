#version 130

in vec3 vp_ms;

uniform mat4 MVP;
uniform vec4 Tint;
out vec4 frag_color;

void main(){
	gl_Position.x = vp_ms.x;
	gl_Position.y = vp_ms.z;
	gl_Position.z = vp_ms.y;
	gl_Position.w = 1.0;
    gl_Position = MVP * gl_Position;
	frag_color = vec4(1.0, 1.0, 1.0, 1.0) * Tint;
}

