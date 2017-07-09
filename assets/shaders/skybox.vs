#version 130

attribute vec3 pos;

uniform mat4 _perspMatrix;
uniform mat4 _cameraMatrix;

out vec3 texCoord;

void main()
{
	texCoord = pos;
	vec4 cameraPos = _cameraMatrix * vec4(0.0,0.0,0.0,1.0);
	cameraPos.w = 0;
    gl_Position = _perspMatrix * (_cameraMatrix * vec4(100 * pos, 1.0) - cameraPos);
}