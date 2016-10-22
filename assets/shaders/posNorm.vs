#version 120

attribute vec3 pos;
attribute vec3 normal;
attribute vec2 uv;

uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;

//varying vec2 outUv;
//varying vec3 outNormal;
//varying vec3 outPos;

void main(){
	//outUv = uv;
	//outNormal = normal;
	//outPos = pos;
	
	vec4 _pos = vec4(pos, 1);
	gl_Position = _perspMatrix * _camMatrix * _objMatrix * _pos;
}
