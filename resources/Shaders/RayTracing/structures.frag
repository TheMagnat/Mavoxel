
//Structures definition
struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
};

//TODO: remove sun, create light with it's attributes
struct LightVoxel {
    vec3 position;
};

struct SimpleVoxel {
    vec3 position;
    vec3 extents;
    Material material;
};
