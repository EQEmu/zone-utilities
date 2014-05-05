#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

uniform mat4 MVP;
uniform vec3 Tint;
out vec3 fragmentColor;

void main(){
	gl_Position.x = vertexPosition.x;
	gl_Position.y = vertexPosition.z;
	gl_Position.z = vertexPosition.y;
	gl_Position.w = 1.0;
    gl_Position = MVP * gl_Position;
	fragmentColor = vertexColor * Tint;
}

