#version 120

attribute vec3 pos;
attribute vec2 uv;

uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;

uniform float _time = 0.0;

uniform float waveHeight = 0.03;
uniform float wavePeriod = 0.3;

varying vec2 outUv;

void main(){
	vec3 vpos = pos;
	vpos.y += sin((pos.x * pos.z + pos.x + pos.z) * _time * wavePeriod) * waveHeight;
	vec4 outPos = vec4(vpos, 1);
	outUv = uv;
	gl_Position = _perspMatrix * _camMatrix * _objMatrix * outPos;
}
