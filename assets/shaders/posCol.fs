#version 120

//varying vec2 outUv;

//uniform sampler2D _mainTex;

uniform vec4 color;

void main(){
	gl_FragColor = color;// texture2D(_mainTex, vec2(0.2, 0.2));
}
