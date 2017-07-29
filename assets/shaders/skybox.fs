#version 130

out vec4 FragColor;

uniform samplerCube _cubeMap;

in vec3 texCoord;

void main()
{
	vec3 texCoordTrans = texCoord;
	texCoordTrans.y = -texCoordTrans.y;
    FragColor = texture(_cubeMap, texCoordTrans/2);
}