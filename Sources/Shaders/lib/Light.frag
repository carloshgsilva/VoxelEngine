#ifndef LIGHT_H
#define LIGHT_H

// requires: 
//  #define IMPORT
//  push_constant BVHBufferRID;
//  push_constant BVHLeafBufferRID;

#include "Common.frag"
#include "Math.frag"

#ifndef IMPORT
const int BVHBufferRID = 0;
const int BVHLeafsBufferRID = 0;
const int TLASRID = 0;
#endif

struct BVHLeaf {
    vec3 right;    uint packedSize;
    vec3 up;       int PADDING;
    vec3 forward;  int palleteID;
    vec3 position; int volumeRID;
};

const int MAX_BVH_LEAF_COUNT = 1024;
BINDING_BUFFER_R(BVHLeafsBuffer, 
	BVHLeaf leafs[MAX_BVH_LEAF_COUNT];
)

struct BVHNode {
	vec3 min;
	int second; //Points to the second child BVHNode
	vec3 max;
	int leaf; //If different from zero this is an leaf and contains the entity id
};

const int MAX_BVH_COUNT = 16384;
BINDING_BUFFER_R(BVHBuffer, 
	BVHNode nodes[MAX_BVH_COUNT];
)

bool volumeSparseRaycastDistance(vec3 origin, vec3 dir, out vec3 hit, int volumeRID){
    float stepFactor = 0.5;
    vec3 stepDir = dir * stepFactor;

    vec3 pos = origin;
    float d = stepFactor;
    
    while(d < 256.0){
        uint voxel = texelFetch(USampler3D[volumeRID], ivec3(pos), 0).r;

        if(voxel >= 8) {
            hit = origin+dir*d;
            return true;
        }
        pos += stepDir;
        d += stepFactor;
        stepFactor *= 1.01;
        stepDir = dir * stepFactor;
    }
    return false;
}

bool intersectVolume(vec3 origin, vec3 direction, out vec3 hit, out vec3 normal, out uint material, int volumeRID){
    ivec3 u_VolumeDimension = textureSize(USampler3D[volumeRID], 0);
    if(abs(direction.x) <= 1e-16) direction.x = EPS;
    if(abs(direction.y) <= 1e-16) direction.y = EPS;
    if(abs(direction.z) <= 1e-16) direction.z = EPS;

    vec3 stepSign = sign(direction);
    vec3 t_delta = 1.0/(direction*stepSign);

    int mip = 0;
    
    int i = 0, nt = 0;
    do{
        float mipSize = float(1 << mip);

        origin /= mipSize;
        ivec3 current_voxel = ivec3(floor(origin));
        vec3 next_voxel_bounds = vec3(current_voxel) + (stepSign*0.5 + 0.5);
        vec3 t_max = (next_voxel_bounds - origin)/direction;
        
        int n = 0;
        
        do{
            vec3 select = step(t_max.xyz, t_max.zxy) * step(t_max.xyz, t_max.yzx);
            current_voxel += ivec3(select*stepSign);
            
            if(clamp(current_voxel, ivec3(-1), ivec3(u_VolumeDimension/mipSize)+1) != current_voxel){ return false; }

            uint voxel = texelFetch(USampler3D[volumeRID], ivec3(current_voxel), mip).r;
            
            
            if(voxel >= 15u){
            float best_t = dot(t_max, select);
                if(mip == 0 
                    #if LOD
                        || mip < 0.003*distance(In.localCameraPos, (origin + direction*best_t)*mipSize)
                    #endif
                ){
                    normal = -stepSign * select;
                    hit = (origin + direction*best_t)*mipSize;
                    material = voxel;
                    return true;
                } else {
                    mip--;
                    origin = origin + direction*best_t/mipSize;
                    break;
                }
            }
            
            t_max += t_delta * select;
            nt++;
        }while(++n < 512);

        origin *= mipSize;

    }while(++i < 4);
    return false;
}

struct TraceHit {
    float t;
    uint visibility;
    int objectId;
    vec3 normal;

    vec3 debug;
};

bool RayTrace(vec3 o, vec3 d, inout TraceHit hit, float t) {
    hit.debug = vec3(0);
    hit.t = t;

    int current_node = 0;
    int visit_next_index = -1;
    int visit_next[32];

    while(true) {
        BVHNode node = BVHBuffer[BVHBufferRID].nodes[current_node];

        float aabb_t = 0.0;
        if(IntersectRayAABB(o, d, node.min, node.max, aabb_t) && aabb_t < hit.t) {
            //hit.debug += vec3(0.01);

            if(node.leaf != -1){
                BVHLeaf leaf = BVHLeafsBuffer[BVHLeafsBufferRID].leafs[node.leaf];
                mat4 mat = mat4(
                    vec4(leaf.right, 0),
                    vec4(leaf.up, 0),
                    vec4(leaf.forward, 0),
                    vec4(leaf.position, 1)
                );
                vec3 size = vec3((leaf.packedSize >> 0) & 0x3FFu, (leaf.packedSize >> 10) & 0x3FFu , (leaf.packedSize >> 20) & 0x3FFu)*0.1;

                mat4 inv_mat = inverse(mat);
                vec3 local_o = (inv_mat * vec4(o, 1.0)).xyz;
                vec3 local_d = (inv_mat * vec4(d, 0.0)).xyz;

                float local_t = 0.0;
                if(IntersectRayAABB(local_o, local_d, vec3(0), vec3(size), local_t)) {
                    //hit.debug.y += 0.05;

                    local_o += local_d * (local_t-EPS);
                    vec3 hitPos;
                    vec3 outNormal = vec3(0.0);
                    uint outMat = 0;
                    if(intersectVolume(local_o*10.0, local_d, hitPos, outNormal, outMat, leaf.volumeRID)){
                        //hit.debug.z += 0.5;
                        float v_t = dot((mat*vec4(hitPos*0.1, 1.0)).xyz - o, d);
                        if(v_t < hit.t) {
                            hit.debug = fract(vec3(outMat)*vec3(314.412, 642.451, 41251.671));
                            hit.t = v_t;
                            hit.visibility = PackVisiblity(leaf.palleteID, outMat);
                            hit.normal = (mat*vec4(outNormal, 0)).xyz;
                        }
                    }
                }
            }else{
                visit_next[++visit_next_index] = node.second;
                current_node++;
                continue;
            }
        }

        if(visit_next_index == -1){
            break;
        }else{
            current_node = visit_next[visit_next_index--];
        }
    }
    return hit.t < t;
}

bool RayTraceShadow(vec3 o, vec3 d, float t) {
    TraceHit hit;
    return RayTrace(o, d, hit, t);
}

bool TraceShadowRay(vec3 origin, vec3 dir, float tmax) {
    float tmin = 0.0;
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, TLAS[TLASRID], gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tmin, dir, tmax);
    while(rayQueryProceedEXT(rayQuery)){
        if(rayQueryGetIntersectionTypeEXT(rayQuery, false) == gl_RayQueryCandidateIntersectionAABBEXT) {
            rayQueryGenerateIntersectionEXT(rayQuery, 0.0);
        }
    }
    return rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT;
}


#endif