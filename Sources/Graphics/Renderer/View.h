#pragma once

#include "glm/glm.hpp"
#include "Core/Core.h"

// Represents the view of a camera
class View {
public:
	glm::vec3 Position;
	glm::mat4 CameraMatrix;
	glm::mat4 ViewMatrix;//Inverse of CameraMatrix
	glm::mat4 ProjectionMatrix;
	uint32 Width, Height;
};

// The raw data needed for the GPU to render
struct ViewData {
	glm::mat4 LastViewMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 InverseViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::mat4 InverseProjectionMatrix;
	glm::vec2 Res;
	glm::vec2 iRes;
	glm::vec3 CameraPosition; int _PADDING_0;
	glm::vec2 Jitter;
	int Frame;
	int ColorTextureRID;
	int DepthTextureRID;
	int PalleteColorRID;
	int PalleteMaterialRID;
};