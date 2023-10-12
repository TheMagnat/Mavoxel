

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    vec2 sun;
};

layout (set = 0, binding = 1) uniform FilterInformations {
    mat4 oldProjectionViewMat;
    mat4 newProjectionViewMat;
    mat4 view;
    float velocityScalar;
    int debug;
};