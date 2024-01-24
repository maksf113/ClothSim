#version 450 core
#extension EXT_gpu_shader4 : require

precision highp float;


layout (location = 0) in vec4 inCrntPos;
layout (location = 1) in vec4 inPrevPos;

uniform samplerBuffer crntPosTex;
uniform samplerBuffer prevPosTex;
uniform int texSizeX;
uniform int texSizeZ;
uniform int iterations;

uniform vec2 invClothSize; // size of a single patch in world space

uniform float ksStr = 150.75, ksShr = 40.75, ksBnd = 80.95, 
			  kdStr = -0.05, kdShr = -0.05, kdBnd = -0.05, 
			  DEFAULT_DAMPING = -0.001;
uniform vec3 wind = vec3(0.0, 0.0, 4.0);

uniform vec3 gravity = vec3(0.0, -9.81, 0.0);
uniform float mass = 1.0;

out vec4 outCrntPos;
out vec4 outPrevPos;
out vec3 outNormal;

uniform mat4 camMatrix;
uniform mat4 model;

const float dt = 1.0 / 60.0 / iterations;

vec3 CalcNormal()
{
	vec3 normal = vec3(0.0);
	int index = gl_VertexID;
	// i -> x , j -> z
	int i = index % texSizeX;
	int j = index / texSizeX;
	vec3 left, right, up, down;

	if(i - 1 >= 0)
		left = texelFetchBuffer(crntPosTex, j * texSizeX + i - 1).xyz;
	else left = inCrntPos.xyz;

	if(i + 1 < texSizeX)
		right = texelFetchBuffer(crntPosTex, j * texSizeX + i + 1).xyz;
	else right = inCrntPos.xyz;

	if(j - 1 >= 0)
		up = texelFetchBuffer(crntPosTex, (j - 1) * texSizeX + i).xyz;
	else
		up = inCrntPos.xyz;

	if(j + 1 < texSizeZ)
		down = texelFetchBuffer(crntPosTex, (j + 1) * texSizeX + i).xyz;
	else
		down = inCrntPos.xyz;

	normal = cross((up- down), (left - right));

	return normalize(normal);
}

ivec2 GetNextNeighbour(int n, out float ks, out float kd)
{
	// structural
	if(n < 4)
	{
		ks = ksStr;
		kd = kdStr;
	}
	if(n == 0)	return ivec2(1, 0);
	if(n == 1)	return ivec2(0, -1);
	if(n == 2)  return ivec2(-1, 0);
	if(n == 3)	return ivec2(0, 1);

	// shear
	if(n < 8)
	{
		ks = ksShr;
		kd = kdShr;
	}
	if( n == 4) return ivec2(1, -1);
	if( n == 5) return ivec2(-1, -1);
	if( n == 6) return ivec2(-1, 1);
	if( n == 7) return ivec2(1, 1);

	// bend
	if(n < 12)
	{
		ks = ksBnd;
		kd = kdBnd;
	}
	if(n == 8)	return ivec2(2, 0);
	if(n == 9)	return ivec2(0, -2);
	if(n == 10)  return ivec2(-2, 0);
	if(n == 11)	return ivec2(0, 2);
}

void main()
{
	float m = inCrntPos.w;
	vec3 crntPos = inCrntPos.xyz;
	vec3 prevPos = inPrevPos.xyz;
	vec3 vel = (crntPos - prevPos) / dt;

	float ks = 0;
	float kd = 0;
	int index = gl_VertexID;
	// i -> x , j -> z
	int i = index % texSizeX;
	int j = index / texSizeX;

	if(index == 0 || index == texSizeX - 1)
		m = 0;

	outNormal = CalcNormal();
	vec3 F = gravity * m + DEFAULT_DAMPING * (vel - wind * outNormal.z);

	for(int n = 0; n < 12; n++)
	{
		// relative neighbour coordinate
		ivec2 nRelCoord = GetNextNeighbour(n, ks, kd);
		int ni = nRelCoord.x;
		int nj = nRelCoord.y;
		if(i + ni < 0 || i + ni > texSizeX - 1)
			continue;
		if(j + nj < 0 || j + nj > texSizeX - 1)
			continue;
		// neighbour index , position, velocity
		int nIndex = (j + nj) * texSizeX + i + ni;
		vec3 nCrntPos = texelFetchBuffer(crntPosTex, nIndex).xyz;
		vec3 nPrevPos = texelFetchBuffer(prevPosTex, nIndex).xyz;

		float restLength = length(invClothSize * nRelCoord);
		
		vec3 nVel = (nCrntPos - nPrevPos) / dt;
		vec3 deltaPos = crntPos - nCrntPos;
		float dist = length(deltaPos);
		vec3 deltaVel = vel - nVel;

		float leftTerm = -ks * (dist - restLength);
		float rightTerm = kd * (dot(deltaVel, deltaPos) / dist);
		vec3 springForce = (leftTerm + rightTerm) * normalize(deltaPos);

		F += springForce;
	}

	vec3 acc = vec3(0.0);
	 if(m != 0)
		acc = F / m;

	// Verlet integration
	vec3 temp = crntPos;
	crntPos = 2.0 * crntPos - prevPos + acc * dt * dt;
	prevPos = temp;

	outCrntPos = vec4(crntPos, m);
	outPrevPos = vec4(prevPos, m);
	gl_Position = camMatrix * model * vec4(crntPos, 1.0);
}

