#version 120

#define MAX_BONE_COUNT 24

attribute vec3 pos;
attribute vec3 normal;
attribute vec2 uv;

attribute vec4 boneWeights;
attribute vec4 boneIndices;

uniform mat4 _objMatrix;
uniform mat4 _camMatrix;
uniform mat4 _perspMatrix;

uniform mat4 _armatureMatrices[MAX_BONE_COUNT];

varying vec2 outUv;
varying vec3 outNormal;
varying vec3 outPos;

void main(){
	mat4 boneMatrix = mat4(0.0);
	for(int i = 0; i < 4; i++){
		mat4 singleBoneMat = _armatureMatrices[int(boneIndices[i])] * boneWeights[i];
		boneMatrix += singleBoneMat;
	}

	mat4 boneObjMat = _objMatrix * boneMatrix;
	
	outUv = uv;
	outNormal = (boneObjMat * vec4(normal, 0.0)).xyz;
	outPos = (boneMatrix * vec4(pos, 1.0)).xyz;
	
	vec4 _pos = vec4(pos, 1);
	gl_Position = _perspMatrix * _camMatrix * boneObjMat * _pos;
}
