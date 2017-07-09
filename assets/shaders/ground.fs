#version 120

varying vec2 outUv;
varying vec3 outNormal;
varying vec3 outPos;

uniform sampler2D _mainTex;

uniform vec3 _lightAngle = vec3(0, 0.7, 0.7);

uniform float _ambientAmount = 0.2;

uniform float tilingScale = 14.0;
uniform float secondLayerScale = 7.0;
uniform float desaturation = 0.25;
uniform float brightness = 2.0;

vec4 rgb2grey(vec4 col){
	float val = dot(col.rgb, vec3(0.22, 0.707, 0.071));
	return vec4(val, val, val, 1.0);
}

void main(){
	float lightDot = dot(outNormal, _lightAngle);
	float _lightFactor = clamp(lightDot, 0, 1);
	float lightFactor = _lightFactor + _ambientAmount;
	
	vec4 texCol1 = texture2D(_mainTex, outUv * tilingScale);
	vec4 texCol2 = texture2D(_mainTex, outUv * tilingScale / -secondLayerScale);
	vec4 texCol = texCol1 * mix(texCol2, rgb2grey(texCol2), desaturation) * brightness;
	
	gl_FragColor = texCol * clamp(lightFactor, 0, 1);
}