

// requires: 
//  #define IMPORT
//  push_constant _ShadowVoxRID;

#include "Common.frag"

#ifndef IMPORT
const int _ShadowVoxRID = 0;
#endif


bool getVolumeAt(ivec3 pos, int mip){
	int bit = (pos.x & 1) | ((pos.y & 1) << 1) | ((pos.z & 1) << 2);
	int mask = 1 << bit;
    pos /= 2;

    int realMip = mip/2;
    uint voxel = texelFetch(SHADOW_VOX_TEXTURE, pos, realMip).r;

    if(mip % 2 == 1){
        return voxel != 0;
    } else {
        return (voxel & mask) != 0;
    }
}

bool raycastShadowVolume(vec3 origin, vec3 direction, float maxt, out vec3 hit, out vec3 normal){
    ivec3 u_VolumeDimension = textureSize(SHADOW_VOX_TEXTURE, 0);

    vec3 stepSign = sign(direction);
    vec3 t_delta = 1.0/(direction*stepSign);

    int mip = 0;
    

    int i = 0, nt = 0;
    float totalt = 0.0;
    do{
        float mipSize = float(1 << mip);

        origin /= mipSize;
        ivec3 current_voxel = ivec3(floor(origin));
        vec3 next_voxel_bounds = vec3(current_voxel) + (stepSign*0.5 + 0.5);
        vec3 t_max = (next_voxel_bounds - origin)/direction;
        
        int n = 0;
        
        do{
            vec3 select = step(t_max.xyz, t_max.zxy) * step(t_max.xyz, t_max.yzx);
            
            if(clamp(current_voxel, ivec3(0), ivec3(u_VolumeDimension/mipSize)*2+1) != current_voxel){ return false; }

            bool voxel = getVolumeAt(ivec3(current_voxel), mip);
            
            float best_t = dot(t_max, select);
            if(voxel){
                
                if(mip == 0){
                    normal = -stepSign * select;
                    hit = (origin + direction*best_t)*mipSize;
                    return true;
                } else {
                    mip--;
                    origin = origin + direction*best_t/mipSize;
                    break;
                }
            }
            
            current_voxel += ivec3(select*stepSign);
            t_max += t_delta * select;
            totalt = best_t;
            nt++;
        }while(++n < 256 && totalt < maxt);

        origin *= mipSize;

    }while(++i < 4);
    return false;
}
/*
float raycastShadowVolumeSparse(vec3 origin, vec3 dir, float dist) {
    float stepFactor = 0.5;
    vec3 stepDir = dir * stepFactor;

    vec3 pos = origin;
    float d = stepFactor;

    int lod = 0;
    int i=0;//TODO: Remove me
    while(d < dist && i++ < 200) {
        uint vol = texelFetch(SHADOW_VOX_TEXTURE, ivec3(pos/2), 0).r;
        
        if(lod == 0){
            uint bit = 0u;
            bit += mod(pos.x, 0.5) > 0.25 ? 1u : 0u;
            bit += mod(pos.y, 0.5) > 0.25 ? 2u : 0u;
            bit += mod(pos.z, 0.5) > 0.25 ? 4u : 0u;
            uint mask = 1u << bit;
            bool voxel = (mask & vol) != 0;

            if(voxel){
                return d;
            }else{
                if(vol == 0){
                    lod = 1;
                    stepDir *= 2.0;
                    stepFactor *= 2.0;
                }
            }

            
        }else if(lod == 1){
            if(vol != 0){
                lod = 0;
                stepDir *= 0.5;
                stepFactor *= 0.5;
                //pos -= stepDir;
                //d -= stepFactor;
            }
        }
        
        pos += stepDir;
        d += stepFactor;
    }

    return dist;
}
*/
float raycastShadowVolumeSparse(vec3 origin, vec3 dir, float dist) {
    float stepFactor = 0.5;
    vec3 stepDir = dir * stepFactor;

    vec3 pos = origin;
    float d = stepFactor;

    while(d < 16.0){
        #if 0
            bool voxel = getVolumeAt(ivec3(pos), 0);
        #else
            uint vol = texelFetch(SHADOW_VOX_TEXTURE, ivec3(pos/2), 0).r;
            uint bit = 0u;
            bit += mod(pos.x, 0.5) > 0.25 ? 1u : 0u;
            bit += mod(pos.y, 0.5) > 0.25 ? 2u : 0u;
            bit += mod(pos.z, 0.5) > 0.25 ? 4u : 0u;
            uint mask = 1u << bit;
            bool voxel = (mask & vol) != 0;
        #endif

        if(voxel) {
            return d;
        }
        pos += stepDir;
        d += stepFactor;
    }

    stepFactor *= 2.0;
    stepDir *= 2.0;
    float lod1MaxT = min(dist, 164.0);
    while(d < lod1MaxT){//TODO: lower this when implemented New Lod
        bool voxel = getVolumeAt(ivec3(pos), 1);
        if(voxel)
            return d;
        
        pos += stepDir;
        d += stepFactor;
    }

    //TODO: New Lod

    return dist;
}

float raycastShadowVolumeSuperSparse(vec3 origin, vec3 dir, float dist) {
    float stepFactor = 2.5;
    vec3 stepDir = dir * stepFactor;

    vec3 pos = origin;
    float d = stepFactor;

    while(d < 16.0){
        #if 0
            bool voxel = getVolumeAt(ivec3(pos), 0);
        #else
            uint vol = texelFetch(SHADOW_VOX_TEXTURE, ivec3(pos/2), 0).r;
            uint bit = 0u;
            bit += mod(pos.x, 0.5) > 0.25 ? 1u : 0u;
            bit += mod(pos.y, 0.5) > 0.25 ? 2u : 0u;
            bit += mod(pos.z, 0.5) > 0.25 ? 4u : 0u;
            uint mask = 1u << bit;
            bool voxel = (mask & vol) != 0;
        #endif

        if(voxel) {
            return d;
        }
        pos += stepDir;
        d += stepFactor;
    }

    stepFactor *= 2.0;
    stepDir *= 2.0;
    float lod1MaxT = min(dist, 164.0);
    while(d < lod1MaxT){//TODO: lower this when implemented New Lod
        bool voxel = getVolumeAt(ivec3(pos), 1);
        if(voxel)
            return d;
        
        pos += stepDir;
        d += stepFactor;
    }

    //TODO: New Lod

    return dist;
}