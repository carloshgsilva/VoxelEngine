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

bool volumeSparseRaycastDistance(vec3 origin, vec3 dir, out vec3 hit, vec3 volumeDimension, int volumeRID){
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
                        vec3 pos = pos + dir*(t-0.0001);
                        vec3 hitPos;
                        mat4 invMat = inverse(mat);
                        vec3 rayPos = (invMat*vec4(pos,1)).xyz*10.0;
                        if(volumeSparseRaycastDistance(rayPos, normalize((invMat*vec4(dir,0)).xyz), hitPos, size, leaf.volumeRID)){
                            best_t = min(best_t, t + distance(rayPos, hitPos));
                        }
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