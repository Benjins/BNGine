out vec4 FragColor;

in vec2 outUv;

uniform sampler2D _mainTex;

void main(){
	FragColor = texture2D(_mainTex, outUv);
}