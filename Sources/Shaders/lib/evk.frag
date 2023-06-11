#ifndef EVK_H
#define EVK_H

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_image_load_formatted  : require
#extension GL_EXT_shader_atomic_float  : require

//#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#extension GL_EXT_debug_printf : enable
#define printf debugPrintfEXT

#define RID int
#define STATE(x)
#define CONSTANT(id) layout(constant_id = id) const
#define PUSH(code) layout(push_constant) uniform _PushConstant { code };
#define IN(n) layout(location=n) in
#define OUT(n) layout(location=n) out
#define COMPUTE(x, y, z) layout(local_size_x = x, local_size_y = y, local_size_z = z) in;

#ifdef VERTEX
	#define V2F(x)  \
	layout(location=0) out struct { \
		x           \
	} Out;
#endif

#ifdef FRAGMENT
	#define V2F(x)  \
	layout(location=0) in struct {\
		x           \
	} In;
#endif

#define BINDING_BUFFER_RW(name, code) layout(binding = 0) restrict volatile coherent buffer name##_t { code } name[];
#define BINDING_BUFFER_R(name, code) layout(binding = 0) restrict readonly buffer name##_t { code } name[];
#define BINDING_BUFFER_W(name, code) layout(binding = 0) restrict writeonly buffer name##_t { code } name[];

layout(binding = 1) uniform sampler2D Sampler2D[];
layout(binding = 1) uniform usampler2D USampler2D[];
layout(binding = 1) uniform sampler3D Sampler3D[];
layout(binding = 1) uniform usampler3D USampler3D[];
layout(binding = 1) uniform samplerCube SamplerCube[];

layout(binding = 2) uniform restrict writeonly image2D Image2DW[];
layout(binding = 2) uniform restrict writeonly iimage2D IImage2DW[];
layout(binding = 2) uniform restrict writeonly uimage2D UImage2DW[];
layout(binding = 2) uniform restrict writeonly image3D Image3DW[];
layout(binding = 2) uniform restrict writeonly iimage3D IImage3DW[];
layout(binding = 2) uniform restrict writeonly uimage3D UImage3DW[];
layout(binding = 2) uniform restrict readonly image2D Image2DR[];
layout(binding = 2) uniform restrict readonly iimage2D IImage2DR[];
layout(binding = 2) uniform restrict readonly uimage2D UImage2DR[];
layout(binding = 2) uniform restrict readonly image3D Image3DR[];
layout(binding = 2) uniform restrict readonly iimage3D IImage3DR[];
layout(binding = 2) uniform restrict readonly uimage3D UImage3DR[];

layout(binding = 3) uniform accelerationStructureEXT TLAS[];

#endif