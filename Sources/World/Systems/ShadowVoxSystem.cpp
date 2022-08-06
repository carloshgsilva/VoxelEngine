#include "ShadowVoxSystem.h"

#include "World/Components.h"
#include "World/Systems/TransformSystem.h"
#include "Profiler/Profiler.h"

void ShadowVoxSystem::OnVoxDestroyed(entt::registry& r, entt::entity e) {
	AssetRefT<VoxAsset>& v = r.get<VoxRenderer>(e).Vox;
	if (!v.IsValid())return;
	Transform& t = r.get<Transform>(e);

	Extent extent = GetDesc(v->GetImage()).extent;
	int sx = extent.width;
	int sy = extent.height;
	int sz = extent.depth;

	glm::ivec3 startmin = glm::vec3(extent.width - 1, extent.height - 1, extent.depth - 1);
	glm::ivec3 startmax = glm::vec3(0, 0, 0);

	glm::ivec3 aabbmin = startmin;
	glm::ivec3 aabbmax = startmax;

	glm::vec3 o = t.WorldMatrix[3];
	glm::vec3 dx = t.WorldMatrix[0] * 0.1f;
	glm::vec3 dy = t.WorldMatrix[1] * 0.1f;
	glm::vec3 dz = t.WorldMatrix[2] * 0.1f;


	for (int z = 0; z < sz; z++) {
		for (int y = 0; y < sy; y++) {
			for (int x = 0; x < sx; x++) {
				if (*v->PixelAt(x, y, z) >= 16) {
					glm::vec3 wp = o + (float)(x)*dx + (float)(y)*dy + (float)(z)*dz;
					glm::ivec3 fwp = glm::ivec3(wp * 10.0f);
					aabbmin = glm::min(aabbmin, fwp);
					aabbmax = glm::max(aabbmax, fwp);
					SetVolumeAt(fwp.x, fwp.y, fwp.z, 0);
				}
			}
		}
	}

	//If isn't inside the ShadowVolume AABB
	if (aabbmax != startmax) {
		aabbmin /= 2;
		aabbmax /= 2;

		aabbmin = glm::max(aabbmin, glm::ivec3(0, 0, 0));
		aabbmax = glm::min(aabbmax, startmin);

		glm::ivec3 size = aabbmax - aabbmin + glm::ivec3(1, 1, 1);
		_UpdateRegions.push_back(ImageRegion{ aabbmin.x, aabbmin.y, aabbmin.z, size.x, size.y, size.z, 0 });
	}
}

ShadowVoxSystem::ShadowVoxSystem() {
	_SizeX = 524;
	_SizeY = 188;
	_SizeZ = 524;
	_SizeXtimes_SizeY = _SizeX * _SizeY;

	_Volume = CreateImage({
		.extent = { _SizeX, _SizeY, _SizeZ },
		.format = Format::R8Uint,
		.usage = ImageUsage::Storage | ImageUsage::TransferDst
	});
	uint64_t size = _SizeX * _SizeY * _SizeZ;
	_Buffer = CreateBuffer({
		.size = size, 
		.memoryType = MemoryType::CPU
		});
	_Data.resize(size);

	return;
	//Clear Volume
	for (int i = 0; i < _SizeX*_SizeY*_SizeZ; i++) {
		uint8* data = (uint8*)_Buffer.GetPtr() + i;
		*data = 0;
	} 

	_UpdateRegions.push_back(ImageRegion{0,0,0, (int)_SizeX, (int)_SizeY, (int)_SizeZ, 0});
	CmdBarrier(_Volume, ImageLayout::Undefined, ImageLayout::TransferDst);
	CmdCopy(_Buffer, _Volume, _UpdateRegions);
	CmdBarrier(_Volume, ImageLayout::TransferDst, ImageLayout::ShaderRead);
}


inline void ShadowVoxSystem::SetVolumeAt(int x, int y, int z, int value) {
	if (x < 0 || y < 0 || z < 0 || x >= _SizeX*2 || y >= _SizeY*2 || z >= _SizeZ*2)return;
	
	int bit = (x & 1) | ((y & 1) << 1) | ((z & 1) << 2);
	int mask = 1 << bit;

	x /= 2;
	y /= 2;
	z /= 2;

	uint8* data = ((uint8*)_Buffer.GetPtr()) + ((size_t)x + ((size_t)y * (size_t)_SizeX) + ((size_t)z * (size_t)(_SizeXtimes_SizeY)));
	*data = (*data & ~mask) | (value << bit);
}

inline bool ShadowVoxSystem::GetVolumeAt(int x, int y, int z) {
	Extent extent = GetDesc(_Volume).extent;
	if (x < 0 || y < 0 || z < 0 || x >= extent.width || y >= extent.height || z >= extent.depth)return false;

	int bit = (x & 1) | ((y & 1) << 1) | ((z & 1) << 2);
	int mask = 1 << bit;

	x /= 2;
	y /= 2;
	z /= 2;

	uint8* data = ((uint8*)_Buffer.GetPtr()) + ((size_t)x + ((size_t)y * (size_t)extent.width) + ((size_t)z * (size_t)extent.width * (size_t)extent.height));
	return (*data & mask) != 1;
}

void ShadowVoxSystem::OnCreate() {
	R->prepare<VoxRenderer>();
	R->on_destroy<VoxRenderer>().connect<&ShadowVoxSystem::OnVoxDestroyed>(this);

}

void ShadowVoxSystem::OnUpdate(float dt) {
	return;
	PROFILE_FUNC();

	R->view<Transform, VoxRenderer, Changed>().each([&](const entt::entity e, Transform& t, VoxRenderer& vr) {
		if (!vr.Vox.IsValid())return;

		AssetRefT<VoxAsset>& v = vr.Vox;
		
		int sx = GetDesc(v->GetImage()).extent.width;
		int sy = GetDesc(v->GetImage()).extent.height;
		int sz = GetDesc(v->GetImage()).extent.depth;

		glm::ivec3 startmin = glm::vec3(GetDesc(_Volume).extent.width-1, GetDesc(_Volume).extent.height-1, GetDesc(_Volume).extent.depth-1);
		glm::ivec3 startmax = glm::vec3(0, 0, 0);

		glm::ivec3 aabbmin = startmin;
		glm::ivec3 aabbmax = startmax;

		glm::mat4 previous_matrix = t.PreviousWorldMatrix;
		previous_matrix = glm::translate(previous_matrix, -vr.Pivot);

		glm::vec3 o =  previous_matrix[3];
		glm::vec3 dx = previous_matrix[0] * 0.1f;
		glm::vec3 dy = previous_matrix[1] * 0.1f;
		glm::vec3 dz = previous_matrix[2] * 0.1f;

		for (int z = 0; z < sz; z++) {
			for (int y = 0; y < sy; y++) {
				for (int x = 0; x < sx; x++) {
					if (*v->PixelAt(x, y, z) >= 16) {
						glm::vec3 wp = o + (float)(x) * dx + (float)(y) * dy + (float)(z) * dz;
						glm::ivec3 fwp = glm::ivec3(wp * 10.0f);
						aabbmin = glm::min(aabbmin, fwp);
						aabbmax = glm::max(aabbmax, fwp);
						//*VolumeAt(fwp.x, fwp.y, fwp.z) = 0;
						SetVolumeAt(fwp.x, fwp.y, fwp.z, 0);
					}
				}
			}
		}

		glm::mat4 current_matrix = t.WorldMatrix;
		current_matrix = glm::translate(current_matrix, -vr.Pivot);
		o =  current_matrix[3];
		dx = current_matrix[0] * 0.1f;
		dy = current_matrix[1] * 0.1f;
		dz = current_matrix[2] * 0.1f;

		for (int z = 0; z < sz; z++) {
			for (int y = 0; y < sy; y++) {
				for (int x = 0; x < sx; x++) {
					if (*v->PixelAt(x, y, z) >= 16) {
						glm::vec3 wp = o + (float)(x) * dx + (float)(y) * dy + (float)(z) * dz;
						glm::ivec3 fwp = glm::ivec3(wp * 10.0f);
						aabbmin = glm::min(aabbmin, fwp);
						aabbmax = glm::max(aabbmax, fwp);
						//*VolumeAt(fwp.x, fwp.y, fwp.z) = 1;
						SetVolumeAt(fwp.x, fwp.y, fwp.z, 1);
					}
				}
			}
		}


		//If isn't inside the ShadowVolume AABB
		if (aabbmax != startmax){
			aabbmin /= 2;
			aabbmax /= 2;

			aabbmin = glm::max(aabbmin, glm::ivec3(0, 0, 0));
			aabbmax = glm::min(aabbmax, startmin);

			glm::ivec3 size = aabbmax - aabbmin + glm::ivec3(1, 1, 1);
			_UpdateRegions.push_back(ImageRegion{ aabbmin.x, aabbmin.y, aabbmin.z, size.x, size.y, size.z, 0 });
		}
	});
	
	if (!_UpdateRegions.empty()) {
		CmdBarrier(_Volume, ImageLayout::ShaderRead, ImageLayout::TransferDst);
		CmdCopy(_Buffer, _Volume, _UpdateRegions);
		CmdBarrier(_Volume, ImageLayout::TransferDst, ImageLayout::ShaderRead);
		_UpdateRegions.clear();
	}
}

