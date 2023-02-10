#include "Common.frag"

layout(push_constant) uniform uPushConstant{
    int _ViewBufferRID;
    int _VoxCmdsBufferRID;
    int _BlueNoiseTextureRID;
    int VoxCmdIndex;
};

#define LOD 1
//not Including the mip zero 
#define MIP_COUNT 2
#define TRANSPARENCY 1

#define IMPORT
#include "View.frag"
BINDING_VOX_CMDS_BUFFER()

int _VolumeRID;
ivec3 volumeDimension;

layout(location=0) in struct {
    vec3 localDirection;
    vec3 localCameraPos;
    mat4 MVPMatrix;
} In;
in vec4 gl_FragCoord;
vec2 UV;

layout(location=0) out vec4 out_Color;
layout(location=1) out vec4 out_Normal;
layout(location=2) out vec4 out_Material;
layout(location=3) out vec2 out_Motion;

vec4 getNoise(int s) {
    vec2 res = GetRes();
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((UV+vec2(GOLDEN_RATIO*(mod(GetFrame()+s*5,8)), GOLDEN_RATIO*(mod(GetFrame()+s*7+1,8))))*res)%512, 0);
}
vec4 getNoise(){
    vec2 res = GetRes();
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((UV+vec2(GOLDEN_RATIO*(mod(GetFrame(),16)), GOLDEN_RATIO*(mod(GetFrame()+1,16))))*res)%512, 0);
}
vec4 getNoise(ivec2 uv){
    vec2 res = GetRes();
    return texelFetch(BLUE_NOISE_TEXTURE, ivec2((uv+vec2(GOLDEN_RATIO*(mod(GetFrame(),16)), GOLDEN_RATIO*(mod(GetFrame()+1,16))))*res)%512, 0);
}

vec3 clipToAABB(vec3 origin, vec3 dir, vec3 volumeSize) {
	if (clamp(origin, vec3(0.0), volumeSize) == origin)
		return origin;

	vec3 invDir = vec3(1.0) / dir;
	vec3 sgn = step(dir, vec3(0.0));
	vec3 t = (sgn*volumeSize - origin)*invDir;
    
	float tmin = max(max(t.x, t.y), t.z);

	return origin + dir*(tmin - 0.001);
}


bool intersectVolume(vec3 origin, vec3 direction, out vec3 hit, out vec3 normal, out uint material, vec2 uv){
    vec3 stepSign = sign(direction);
    vec3 t_delta = 1.0/(direction*stepSign);

    int mip = MIP_COUNT;
    
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
            
            if(clamp(current_voxel, ivec3(-1), ivec3(volumeDimension/mipSize)+1) != current_voxel){
                return false;
            }

            uint voxel = texelFetch(VOLUME_TEXTURE, ivec3(current_voxel), mip).r;
            
            if(voxel != 0u){
                    float best_t = dot(t_max, select);
                    if(mip == 0
                        #if LOD
                            || mip < 0.001*distance(In.localCameraPos, (origin + direction*best_t)*mipSize)
                        #endif
                    ){
                        
                    #if TRANSPARENCY
                        vec2 coords = round(uv*GetRes()*0.5);
                        if(!(voxel < 16 && mod(coords.y+coords.x, 2) == (GetFrame())%2)){//If glass
                    #endif
                            normal = -stepSign * select;
                            hit = (origin + direction*best_t)*mipSize;
                            material = voxel;
                            return true;
                    #if TRANSPARENCY
                        }
                    #endif
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

void main() {
    #if 0 //Jelly Mode
        out_Color = vec4(0,1,0, 1.0);
		out_Material = vec4(0);
        out_Normal = vec4(0,1,0,1);
        gl_FragDepth = 1.0; // Add Some Offset to prevent depth Z-Finghting
        return;
    #endif

    VoxCmd cmd = GetVoxCmd(VoxCmdIndex);
    _VolumeRID = cmd.VolumeRID;

    volumeDimension = textureSize(VOLUME_TEXTURE, 0);
    vec4 sd = In.MVPMatrix*vec4(In.localDirection,0);
    UV = sd.xy/sd.w;
    UV += GetJitter()*GetiRes();

    vec3 localDir = normalize(In.localDirection);
    vec3 localPos = clipToAABB(In.localCameraPos, localDir, vec3(textureSize(VOLUME_TEXTURE, 0)));

    vec3 hitPos = vec3(0,0,1);
    vec3 hitNormal;
    uint hitMat;
    bool hit = intersectVolume(localPos, localDir, hitPos, hitNormal, hitMat, UV); 
    

	if(hit) {
		out_Color = vec4(texelFetch(PALLETE_COLOR_TEXTURE, ivec2(hitMat, cmd.PalleteIndex), 0).xyz, step(hitMat, 16));
		out_Material = texelFetch(PALLETE_MATERIAL_TEXTURE, ivec2(hitMat, cmd.PalleteIndex), 0);
        out_Normal = normalize(cmd.WorldMatrix*vec4(hitNormal, 0.0));
        
        vec4 worldHitPos = cmd.WorldMatrix*vec4(hitPos*0.1, 1);
        vec4 lastWorldHitPos = cmd.LastWorldMatrix*vec4(hitPos*0.1, 1);
        vec4 currentUV = (GetProjectionMatrix()*GetViewMatrix()*worldHitPos);
        vec4 lastUV = (GetProjectionMatrix()*GetLastViewMatrix()*lastWorldHitPos);

        ivec2 coords = ivec2(floor((0.5*currentUV.xy/currentUV.w)*GetRes()));

        out_Motion =  -vec2(0.5,-0.5)*((currentUV.xy/currentUV.w - lastUV.xy/lastUV.w));
   
        //Frag Depth
        float linearDepth = currentUV.w;
        gl_FragDepth = (1.0 - 0.0000001*_VolumeRID)*(linearDepth-NEAR)/(FAR-NEAR); // Add Some Offset to prevent depth Z-Finghting
    }else{

    #if 0 //Red Bounding Box
            float linearDepth = (u_ProjectionMatrix*u_ViewMatrix*u_WorldMatrix*vec4(hitPos*0.1, 1)).w;
            gl_FragDepth = (linearDepth-NEAR)/(FAR-NEAR);
            out_Color = vec4(1,0,0,1);
            out_Normal = vec4(0,1,0,0);
            out_Motion = vec2(0,0);
    #else
        discard;
    #endif

    }

}