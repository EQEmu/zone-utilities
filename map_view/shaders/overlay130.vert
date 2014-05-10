#version 130

in vec3 vert_position;
in vec2 vert_uv;

uniform mat4 mvp;

out vec2 uv;

void main(){
	gl_Position = mvp * vec4(vert_position, 1.00);
	
	uv = vert_uv;
	uv.y = -uv.y;
}
