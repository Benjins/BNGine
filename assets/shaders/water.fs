#version 120

varying vec2 outUv;

uniform float uvScale = 1.0;

uniform sampler2D _mainTex;

uniform float alpha = 0.6;

void main(){
	vec4 texCol = texture2D(_mainTex, outUv * uvScale);
	texCol.a = alpha;
	gl_FragColor = texCol;
}

