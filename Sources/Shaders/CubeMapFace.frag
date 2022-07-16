#version 450

#include "lib/Common.frag"

layout(push_constant) uniform uPushConstant{
    int _PanoramaTextureRID;
    int FaceID;
};

layout(location=0) in struct {
    vec2 UV;
} In;

layout(location=0) out vec4 out_Color;

vec3 uvToXYZ(int face, vec2 uv) {
	if(face == 0)
		return vec3(     1.f,   uv.y,    -uv.x);

	else if(face == 1)
		return vec3(    -1.f,   uv.y,     uv.x);

	else if(face == 2)
		return vec3(   +uv.x,   -1.f,    +uv.y);

	else if(face == 3)
		return vec3(   +uv.x,    1.f,    -uv.y);

	else if(face == 4)
		return vec3(   +uv.x,   uv.y,      1.f);

	else //if(face == 5)
	{	return vec3(    -uv.x,  +uv.y,     -1.f);}
}

vec2 dirToUV(vec3 dir) {
	return vec2(
		0.5f + 0.5f * atan(dir.z, dir.x) / MATH_PI,
		1.f - acos(dir.y) / MATH_PI);
}

vec3 panoramaToCubeMap(int face, vec2 texCoord) {
	vec2 texCoordNew = texCoord*2.0-1.0; //< mapping vom 0,1 to -1,1 coords
	vec3 scan = uvToXYZ(face, texCoordNew); 
	vec3 direction = normalize(scan);
	vec2 src = dirToUV(direction);

	return  texture(_BindingSampler2D[_PanoramaTextureRID], src).rgb; //< get the color from the panorama
}

void main() {
    out_Color = vec4(panoramaToCubeMap(FaceID, In.UV), 1.0);
}