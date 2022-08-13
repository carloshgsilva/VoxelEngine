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
#endif


struct BVHLeaf {
    vec3 right;    float sizeX;
    vec3 up;       float sizeY;
    vec3 forward;  float sizeZ;
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
            
            
            if(voxel != 0u){
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
    int objectId;
    ivec3 voxel;
    vec3 normal;
};

float raycastWorldDistance(vec3 pos, vec3 dir, float max_t){
    float best_t = max_t;
    
    int current_node = 0;

    int visit_next_index = -1;
    int visit_next[32];

    for(int i=0;i<100;i++){
        BVHNode node = BVHBuffer[BVHBufferRID].nodes[current_node];
        float nt =  RayCastAABB(pos, dir, node.min, node.max);

        if(nt < best_t){
            if(node.leaf != 0){
                
                BVHLeaf leaf = BVHLeafsBuffer[BVHLeafsBufferRID].leafs[node.leaf];
                mat4 mat = mat4(
                    vec4(leaf.right, 0),
                    vec4(leaf.up, 0),
                    vec4(leaf.forward, 0),
                    vec4(leaf.position, 1)
                );
                vec3 size = vec3(leaf.sizeX, leaf.sizeY, leaf.sizeZ);
            
                float t;
                if(RayOBBCast(mat, size, pos, dir, t)){
                    if(t < best_t){
                        //best_t = t;

                        if(t < 0)t = 0;

                        #if 0

                        vec3 pos = pos + dir*(t-0.0001);
                        vec3 hitPos;
                        mat4 invMat = inverse(mat);
                        vec3 rayPos = (invMat*vec4(pos,1)).xyz*10.0;
                        if(volumeSparseRaycastDistance(rayPos, normalize((invMat*vec4(dir,0)).xyz), hitPos, leaf.volumeRID)){
                            best_t = min(best_t, t + distance(rayPos, hitPos));
                        }

                        #else

                        vec3 pos = pos + dir*(t-0.0001);
                        vec3 hitPos;
                        mat4 invMat = inverse(mat);
                        vec3 rayPos = (invMat*vec4(pos,1)).xyz*10.0;
                        vec3 outNormal = vec3(0.0);
                        uint outMat = 0;
                        if(intersectVolume(rayPos, normalize((invMat*vec4(dir,0)).xyz), hitPos, outNormal, outMat, leaf.volumeRID)){
                            best_t = min(best_t, t + distance(rayPos, hitPos));
                        }

                        #endif
                    }
                }

                //best_t = nt;
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
    return best_t;
}

#endif