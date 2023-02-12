#ifdef _MSC_BUILD
#define uint uint32
using namespace glm;
#endif

struct VoxInstance {
    int palleteIndex;
    int geometryBufferRID;  // also set in the customId of blas instance for fast access
};
/*
struct PointLightData {
    vec3 Position;
    float Range;
    vec3 Color;
    float Attenuation;
};

struct SpotLightData {
    vec3 Position;
    float Range;
    vec3 Color;
    float Attenuation;
    vec3 Direction;
    float Angle;
    float AngleAttenuation;
};
*/