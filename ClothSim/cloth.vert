#version 430 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexUV;

uniform mat4 camMatrix;
uniform mat4 model;

out vec2 texCoord;
out vec3 vNormal;
out vec3 vPos;

void main()
{
	gl_Position = camMatrix * model * vec4(aPosition, 1.0);
	vPos = vec3(model * vec4(aPosition, 1.0));
	vNormal = aNormal;
	texCoord = aTexUV;
}
