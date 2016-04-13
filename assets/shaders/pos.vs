#version 130

attribute vec3 pos;
attribute vec2 uv;

uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;

out vec2 outUv;

void main(){
	vec4 outPos = vec4(pos, 1);
	outUv = uv;
	gl_Position = _perspMatrix * _camMatrix * _objMatrix * outPos;
}