#version 120

//varying vec2 outUv;

uniform sampler2D _mainTex;

void main(){
	gl_FragColor = texture2D(_mainTex, vec2(0.2, 0.2));
}
