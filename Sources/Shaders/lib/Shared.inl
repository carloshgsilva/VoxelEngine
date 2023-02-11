#ifdef _MSC_BUILD
#define uint uint32
using namespace glm;
#endif

struct VoxInstance {
    int palleteIndex;
    int geometryBufferRID;  // also set in the customId of blas instance for fast access
};