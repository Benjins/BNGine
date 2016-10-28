#version 120

varying vec2 outUv;
varying vec3 outNormal;
varying vec3 outPos;

uniform sampler2D _mainTex;

uniform vec3 _lightAngle = vec3(0, 0.7, 0.7);

uniform float _ambientAmount = 0.2;

void main(){
	float lightDot = dot(outPos, _lightAngle);
	float _lightFactor = clamp(lightDot, 0, 1);
	float lightFactor = _lightFactor + _ambientAmount;
	gl_FragColor = texture2D(_mainTex, outUv) * clamp(lightFactor, 0, 1);
}
