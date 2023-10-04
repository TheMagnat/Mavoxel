

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
};

Material light = Material(
    vec3(0.1f, 0.1f, 0.1f),
    vec3(0.5f, 0.5f, 0.5f),
    vec3(1.0f, 1.0f, 1.0f),
    32.0
);

// Material light = Material(
//     vec3(0.1f, 0.1f, 0.1f),
//     vec3(0.5f, 0.5f, 0.0f),
//     vec3(1.0f, 1.0f, 0.0f),
//     32.0
// );

Material materials[] = Material[](
    //0 - Default / Error
    Material(
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //1 - Grass
    Material(
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //2 - Dirt
    Material(
        vec3(0.819f, 0.466f, 0.263f),
        vec3(0.819f, 0.466f, 0.263f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //3 - Stone
    Material(
        vec3(0.7176f, 0.6901f, 0.6117f),
        vec3(0.7176f, 0.6901f, 0.6117f),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //4 - Debug RED
    Material(
        vec3(1.0, 0.0, 0.0),
        vec3(1.0, 0.0, 0.0),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //5 - Debug GREEN
    Material(
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //6 - Debug BLUE
    Material(
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //7 - Debug VIOLET
    Material(
        vec3(1.0, 0.0, 1.0),
        vec3(1.0, 0.0, 1.0),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    ),
    //7 - Debug VIOLET
    Material(
        vec3(1.0, 0.0, 1.0),
        vec3(1.0, 0.0, 1.0),
        vec3(0.1f, 0.1f, 0.1f),
        32.0f
    )
);
