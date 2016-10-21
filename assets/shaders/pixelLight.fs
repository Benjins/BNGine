#version 120

varying vec2 outUv;
varying vec3 outNormal;
varying vec3 outPos;

uniform sampler2D _mainTex;

uniform vec3 _lightAngle;

uniform float _ambientAmount;

void main(){
	float lightDot = dot(outNormal, _lightAngle);
	float lightFactor = clamp(lightDot, 0, 1) + _ambientAmount;
	gl_FragColor = texture2D(_mainTex, outUv) * clamp(lightFactor, 0, 1);
}
