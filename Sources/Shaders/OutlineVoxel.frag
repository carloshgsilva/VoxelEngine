#define LOD 0


layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _VolumeRID;
    int GBufferRID;
    mat4 u_WorldMatrix;
    vec3 u_OutlineColor;
};

#define IMPORT
#include "Common.frag"
#include "View.frag"

layout(location=0) in struct {
    vec3 localDirection;
    vec3 localCameraPos;
} In;

layout(location=0) out vec4 out_Color;

vec3 clipToAABB(vec3 origin, vec3 dir, vec3 volumeSize) {
	if (clamp(origin, vec3(0.0), volumeSize) == origin)
		return origin;

	vec3 invDir = vec3(1.0) / dir;
	vec3 sgn = step(dir, vec3(0.0));
	vec3 t = (sgn*volumeSize - origin)*invDir;
    
	float tmin = max(max(t.x, t.y), t.z);

	return origin + dir*(tmin - 0.001);
}
bool intersectVolume(vec3 origin, vec3 direction, out vec3 hit, out vec3 normal, out uint material){
    ivec3 u_VolumeDimension = textureSize(VOLUME_TEXTURE, 0);

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

            uint voxel = texelFetch(VOLUME_TEXTURE, ivec3(current_voxel), mip).r;
            
            
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

float bestDepth(vec2 uv, vec2 iRes){
    float d = 0.0;
    for(float x=-1;x<=1;x++){
        for(float y=-1;y<=1;y++){
            uvec4 gbuffer = GBufferFetch(GBufferRID, ivec2(uv/iRes)+ivec2(x, y));
            float t = GBufferGetDepth(gbuffer);
            d = max(d, t);
        }
    }
    return d;
}

void main() {
    out_Color = vec4(0,0,0,1);
    
    vec3 localDir = normalize(In.localDirection);
    vec3 localPos = clipToAABB(In.localCameraPos, localDir, vec3(textureSize(VOLUME_TEXTURE, 0)));

    vec3 hitPos = vec3(0,0,1);
    vec3 hitNormal;
    uint hitMat;
    bool hit = intersectVolume(localPos, localDir, hitPos, hitNormal, hitMat);     

	if(hit) {
		out_Color = vec4(u_OutlineColor, 1);
        //Frag Depth
        vec4 projectedPixel = (GetProjectionMatrix()*GetViewMatrix()*u_WorldMatrix*vec4(hitPos*0.1, 1));
        gl_FragDepth = (projectedPixel.w-NEAR)/(FAR-NEAR);

        vec2 iRes = 1.0/textureSize(Sampler2D[GBufferRID], 0);
        vec2 uv = (projectedPixel.xy/projectedPixel.w)*vec2(1,-1)*0.5 + 0.5;
        float sceneDepth = bestDepth(uv, iRes);
        if(gl_FragDepth - sceneDepth > 0.000005){
            out_Color.a = 0.0;
        }
	}else{
        discard;
    }

}