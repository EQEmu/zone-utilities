#version 130

in vec3 vp_ms;
in vec3 vc;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Proj;
uniform vec4 Tint;

out vec4 frag_color;

void main(){
	gl_Position.x = vp_ms.x;
	gl_Position.y = vp_ms.y;
	gl_Position.z = vp_ms.z;
	gl_Position.w = 1.0;
    gl_Position = Proj * View * Model * gl_Position;
	frag_color = vec4(vc.x, vc.y, vc.z, 1.0) * Tint;
}

