#ifndef LIGHT_H
#define LIGHT_H

// requires: 
//  #define IMPORT
//  push_constant TLASRID;
//  push_constant VoxInstancesRID;

#include "Common.frag"
#include "Math.frag"
#include "View.frag"
#include "BRDF.frag"

#ifndef IMPORT
const int TLASRID = 0;
const int VoxInstancesRID = 0;
#endif

bool TraceShadowRay(vec3 origin, vec3 dir, float tmax) {
    float tmin = 0.001;
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, TLAS[TLASRID], gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tmin, dir, tmax);
    while(rayQueryProceedEXT(rayQuery)){
        if(rayQueryGetIntersectionTypeEXT(rayQuery, false) == gl_RayQueryCandidateIntersectionAABBEXT) {
            rayQueryGenerateIntersectionEXT(rayQuery, 0.0);
        }
    }
    return rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT;
}

BINDING_BUFFER_R(VoxGeometry, 
    uint at[];
)

void UnpackVox(uint packedVox, out vec3 aabbMin, out uint matId) {
    uint x = (packedVox >> 0) & 0xFFu;
    uint y = (packedVox >> 8) & 0xFFu;
    uint z = (packedVox >> 16) & 0xFFu;
    matId = (packedVox >> 24) & 0xFFu;
    aabbMin = vec3(x, y, z);
}

uint PackVisiblity(uint instanceId, uint matId) {
    return (instanceId) | (matId << 24);
}
void UnpackVisibility(uint packed, out uint instanceId, out uint matId) {
    instanceId = packed & 0xFFFFFFu;
    matId = packed >> 24;
}

struct TraceHit {
    vec3 normal;
    float t;
    uint visibility;
};

bool TraceRay(vec3 origin, vec3 dir, float tmax, inout TraceHit hit) {
    hit.t = tmax;
    hit.visibility = 0u;
    hit.normal = vec3(0);

    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, TLAS[TLASRID], 0, 0xFF, origin, 0.001, dir, tmax);
    while(rayQueryProceedEXT(rayQuery)){
        if(rayQueryGetIntersectionTypeEXT(rayQuery, false) == gl_RayQueryCandidateIntersectionAABBEXT) {
            int instanceId = rayQueryGetIntersectionInstanceIdEXT (rayQuery, false);
            int geometryRID = rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery, false);
            int primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT (rayQuery, false);
            vec3 origin = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery, false);
            vec3 dir = rayQueryGetIntersectionObjectRayDirectionEXT (rayQuery, false);

            uint packedVox = VoxGeometry[geometryRID].at[primitiveId];
            vec3 aabbMin;
            uint matId;
            UnpackVox(packedVox, aabbMin, matId);

            float t;
            vec3 normal;
            if(IntersectRayAABBNormal(origin, dir, aabbMin, aabbMin + 1.0, t, normal) && t < hit.t) {
                rayQueryGenerateIntersectionEXT(rayQuery, t);
                hit.t = min(hit.t, t);
                hit.visibility = PackVisiblity(instanceId, matId);
                mat4x3 objectToWorld = rayQueryGetIntersectionObjectToWorldEXT(rayQuery, false); // TODO: transform to world normal with visibility buffer
                hit.normal = normalize(vec3(objectToWorld*vec4(normal, 0.0)));
            }
        }
    }
    return rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT;
}

#include "Shared.inl"

BINDING_BUFFER_R(VoxInstances,
    VoxInstance at[];
)

void GetMaterial(uint visibility, inout Material mat) {
    uint instanceId;
    uint matId;
    UnpackVisibility(visibility, instanceId, matId);
    int palleteId = VoxInstances[VoxInstancesRID].at[instanceId].palleteIndex;
    mat.albedo = pow(texelFetch(PALLETE_COLOR_TEXTURE, ivec2(matId, palleteId), 0).xyz, vec3(2.2));
    vec3 rme = texelFetch(PALLETE_MATERIAL_TEXTURE, ivec2(matId, palleteId), 0).xyz;
    mat.roughness = pow(rme.x, 2.2);
    mat.metallic = rme.y;
    mat.emissive = rme.z;
}

#endif