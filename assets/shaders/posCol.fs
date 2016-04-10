out vec4 FragColor;
in vec4 _outPos;
void main(){
	FragColor = (_outPos + vec4(1,1,1,1))/2;
}