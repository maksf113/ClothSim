#version 430 core

in vec2 texCoord;
in vec3 vNormal;
in vec3 vPos;

uniform vec3 color;
uniform sampler2D tex;
uniform vec3 camPos;
//uniform vec3 lightDir;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec3 lightDir = normalize(vec3(0.2, -1.0, 0.3));
	vec3 normal = vNormal;
	vec3 fragFromCam = vPos - camPos;
	if(dot(fragFromCam, normal) > 0.0)
	 normal *= -1.0;
	float ambient = 0.3;
	FragColor = (max(dot(-lightDir, normal), 0.0) + ambient) * texture(tex, texCoord);
}