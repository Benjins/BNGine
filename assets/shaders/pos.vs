#version 130
attribute vec3 pos;
uniform float _x;
uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;
out vec4 _outPos;

void main(){
	vec4 outPos = vec4(pos, 1);
	_outPos = outPos;
	gl_Position = _perspMatrix * _camMatrix * _objMatrix * outPos;
}