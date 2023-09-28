#extension GL_GOOGLE_include_directive : require

#include "materials.frag"
#include "structures.frag"


//Ray casting

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    float xRatio;
    Camera camera;
    mat4 projection;
    mat4 view;
    LightVoxel sun;
    float time;
    vec3 voxelCursorPosition;
    vec3 faceCursorNormal;
    vec2 jitter;
};

layout(std430, binding = 4) buffer SsboSimpleVoxels {
    SimpleVoxel simpleVoxels[];
};


//Octree

//Usefull constants
#define RAYTRACING_CHUNK_PER_AXIS (RAYTRACING_CHUNK_RANGE * 2 + 1)
#define RAYTRACING_SVO_SIZE (RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS * RAYTRACING_CHUNK_PER_AXIS)

#define CENTER_OFFSET ivec3(RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE, RAYTRACING_CHUNK_RANGE)

layout (set = 0, binding = 1) uniform WorldOctreeInformations {
    ivec3 centerChunkPosition;
    int maxDepth;
    int maxLen;
    float voxelSize;
};

layout(std430, binding = 2) buffer SsboOctree {
    int octreeData[];
} ssboOctrees[RAYTRACING_SVO_SIZE];

layout(set = 0, binding = 3) uniform SsboInformations {
    int empty;
} ssboInformations[RAYTRACING_SVO_SIZE];
