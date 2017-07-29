#version 120

varying vec2 outUv;

uniform float uvScale = 1.0;

uniform sampler2D _mainTex;

void main(){
	gl_FragColor = texture2D(_mainTex, outUv * uvScale);
}

