#version 450 core

layout(location = 0) in vec2 vPos;
// layout(location = 0) in vec3 vPos;

uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProjection;
uniform vec4 vColor;

void main() { gl_Position = mProjection * mView * vec4(vPos, 0.0, 1.0); }