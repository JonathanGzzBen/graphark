#version 450 core

layout (location = 0) out vec4 fColor;

// in vec4 vColor;
uniform vec4 vColor;

void main() {
	// fColor = vec4(1.0, 0.5, 0.5, 1.0);
	fColor = vColor;
}