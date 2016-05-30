#version 120

attribute vec3 pos;

uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;

void main(){
	gl_Position = _perspMatrix * _camMatrix * _objMatrix * vec4(pos, 1);
}
