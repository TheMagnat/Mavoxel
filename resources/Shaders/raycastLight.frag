#extension GL_GOOGLE_include_directive : require

#include "octree.frag"
#include "materials.frag"
#include "lightHelper.frag"

#define PI 3.1415926538

//DEBUG TO REMOVE

struct RayAABBResult {
    bool intersect;
    float dist;
};

struct Box {
    vec3 center;
    vec3 radius;
    vec3 invRadius;
    mat3 rotation;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

RayAABBResult box(vec3 ray_origin, vec3 ray_dir, vec3 minpos, vec3 maxpos) {
  vec3 inverse_dir = 1.0 / ray_dir;
  vec3 tbot = inverse_dir * (minpos - ray_origin);
  vec3 ttop = inverse_dir * (maxpos - ray_origin);
  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);
  vec2 traverse = max(tmin.xx, tmin.yz);
  float traverselow = max(traverse.x, traverse.y);
  traverse = min(tmax.xx, tmax.yz);
  float traversehi = min(traverse.x, traverse.y);
  return RayAABBResult(traversehi > max(traverselow, 0.0), traverselow);
}

float maxComponent (vec3 v) {
  return max (max (v.x, v.y), v.z);
}

// vec3 box.radius:       independent half-length along the X, Y, and Z axes
// mat3 box.rotation:     box-to-world rotation (orthonormal 3x3 matrix) transformation
// bool rayCanStartInBox: if true, assume the origin is never in a box. GLSL optimizes this at compile time
// bool oriented:         if false, ignore box.rotation
bool ourIntersectBoxCommon(Box box, Ray ray, out float distance, out vec3 normal, const bool rayCanStartInBox, const in bool oriented, in vec3 _invRayDirection) {

    // Move to the box's reference frame. This is unavoidable and un-optimizable.
    ray.origin = box.rotation * (ray.origin - box.center);
    if (oriented) {
        ray.direction = ray.direction * box.rotation;
    }
    
    // This "rayCanStartInBox" branch is evaluated at compile time because `const` in GLSL
    // means compile-time constant. The multiplication by 1.0 will likewise be compiled out
    // when rayCanStartInBox = false.
    float winding;
    if (rayCanStartInBox) {
        // Winding direction: -1 if the ray starts inside of the box (i.e., and is leaving), +1 if it is starting outside of the box
        winding = (maxComponent(abs(ray.origin) * box.invRadius) < 1.0) ? -1.0 : 1.0;
    } else {
        winding = 1.0;
    }

    // We'll use the negated sign of the ray direction in several places, so precompute it.
    // The sign() instruction is fast...but surprisingly not so fast that storing the result
    // temporarily isn't an advantage.
    vec3 sgn = -sign(ray.direction);

	// Ray-plane intersection. For each pair of planes, choose the one that is front-facing
    // to the ray and compute the distance to it.
    vec3 distanceToPlane = box.radius * winding * sgn - ray.origin;
    if (oriented) {
        distanceToPlane /= ray.direction;
    } else {
        distanceToPlane *= _invRayDirection;
    }

    // Perform all three ray-box tests and cast to 0 or 1 on each axis. 
    // Use a macro to eliminate the redundant code (no efficiency boost from doing so, of course!)
    // Could be written with 
#   define TEST(U, VW)\
         /* Is there a hit on this axis in front of the origin? Use multiplication instead of && for a small speedup */\
         (distanceToPlane.U >= 0.0) && \
         /* Is that hit within the face of the box? */\
         all(lessThan(abs(ray.origin.VW + ray.direction.VW * distanceToPlane.U), box.radius.VW))

    bvec3 test = bvec3(TEST(x, yz), TEST(y, zx), TEST(z, xy));

    // CMOV chain that guarantees exactly one element of sgn is preserved and that the value has the right sign
    sgn = test.x ? vec3(sgn.x, 0.0, 0.0) : (test.y ? vec3(0.0, sgn.y, 0.0) : vec3(0.0, 0.0, test.z ? sgn.z : 0.0));    
#   undef TEST
        
    // At most one element of sgn is non-zero now. That element carries the negative sign of the 
    // ray direction as well. Notice that we were able to drop storage of the test vector from registers,
    // because it will never be used again.

    // Mask the distance by the non-zero axis
    // Dot product is faster than this CMOV chain, but doesn't work when distanceToPlane contains nans or infs. 
    //
    distance = (sgn.x != 0.0) ? distanceToPlane.x : ((sgn.y != 0.0) ? distanceToPlane.y : distanceToPlane.z);

    // Normal must face back along the ray. If you need
    // to know whether we're entering or leaving the box, 
    // then just look at the value of winding. If you need
    // texture coordinates, then use box.invDirection * hitPoint.
    
    if (oriented) {
        normal = box.rotation * sgn;
    } else {
        normal = sgn;
    }
    
    return (sgn.x != 0) || (sgn.y != 0) || (sgn.z != 0);
}


//DEBUG END

//Structures definition
struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
};

//TODO: remove sun, create light with it's attributes
struct Sun {
    vec3 position;
};

//Uniforms
layout (set = 0, binding = 0) uniform RayCastInformations {
    float xRatio;
    Camera camera;
    Sun sun;
    float time;
    vec3 voxelCursorPosition;
    vec3 faceCursorNormal;
};

//TODO: Smooth shadow
bool inShadow (vec3 startPosition, vec3 offset, vec3 lightDir, float lightDist) {
    //Shadow calculation
    WorldRayCastResult shadowRayCastResult = worldCastRay( startPosition + offset, lightDir, lightDist );
    return shadowRayCastResult.voxel != 0;
}

ivec4 edges( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + nor + dir.yzx;
	vec3 v2 = vos + nor - dir.yzx;
	vec3 v3 = vos + nor + dir.zxy;
	vec3 v4 = vos + nor - dir.zxy;

	ivec4 res = ivec4(0);
	if( octreeGetWorld(v1).voxel != 0 ) res.x = 1;
	if( octreeGetWorld(v2).voxel != 0 ) res.y = 1;
	if( octreeGetWorld(v3).voxel != 0 ) res.z = 1;
	if( octreeGetWorld(v4).voxel != 0 ) res.w = 1;

	return res;
}

ivec4 corners( in vec3 vos, in vec3 nor, in vec3 dir )
{
	vec3 v1 = vos + nor + dir.yzx + dir.zxy;
	vec3 v2 = vos + nor - dir.yzx + dir.zxy;
	vec3 v3 = vos + nor - dir.yzx - dir.zxy;
	vec3 v4 = vos + nor + dir.yzx - dir.zxy;

	ivec4 res = ivec4(0);
	if( octreeGetWorld(v1).voxel != 0 ) res.x = 1;
	if( octreeGetWorld(v2).voxel != 0 ) res.y = 1;
	if( octreeGetWorld(v3).voxel != 0 ) res.z = 1;
	if( octreeGetWorld(v4).voxel != 0 ) res.w = 1;

	return res;
}

//Note: UV are like the texture position of the hitted voxel face
vec2 computeHitUV(vec3 hitPos, vec3 hitDir, vec3 hitVoxelChunkPos) {
    vec3 uvw = (hitPos - (hitVoxelChunkPos + voxelSize * 0.5)) / voxelSize;
    return vec2( dot(hitDir.yzx, uvw), dot(hitDir.zxy, uvw) ) + 0.5;
}

//TODO: verify impact on performance of AO
float computeAO(vec3 norm, vec3 hitDir, vec3 hitVoxelChunkPos, vec2 uv) {

    ivec4 ed = edges( hitVoxelChunkPos, norm, hitDir );
    ivec4 co = corners( hitVoxelChunkPos, norm, hitDir );

    float occ = 0.0; 
    vec4 allCorners = vec4(0.0);

    //TODO: si les 2 edge sont true, ça doit valoir 3
    allCorners.x = ed.x + ed.z; //Haut droite
    allCorners.y = ed.z + ed.y; //Bas droite
    allCorners.z = ed.y + ed.w; //Bas gauche
    allCorners.w = ed.w + ed.x; //Haut gauche

    allCorners.x += allCorners.x == 2 ? 1 : co.x; //Haut droite
    allCorners.y += allCorners.y == 2 ? 1 : co.y; //Bas droite
    allCorners.z += allCorners.z == 2 ? 1 : co.z; //Bas gauche
    allCorners.w += allCorners.w == 2 ? 1 : co.w; //Haut gauche


    // vec4 curve = vec4(0.0, 0.6, 0.8, 1.0);
    // allCorners.x = curve[int(allCorners.x)];
    // allCorners.y = curve[int(allCorners.y)];
    // allCorners.z = curve[int(allCorners.z)];
    // allCorners.w = curve[int(allCorners.w)];

    float middleDown = mix(allCorners.z, allCorners.y, uv.y);
    float middleUp = mix(allCorners.w, allCorners.x, uv.y);
    float vertical = mix(middleDown, middleUp, uv.x);

    float middleLeft = mix(allCorners.z, allCorners.w, uv.x);
    float middleRight = mix(allCorners.y, allCorners.x, uv.x);
    float lateral = mix(middleLeft, middleRight, uv.y);

    return 1.0 - (vertical/3.0 * lateral/3.0) * 0.8; //TODO: rendre cette valeur parametrable
}

vec3 applyFog(vec3 color, vec3 fogColor, float colorDistance, float maxDistance) {

    //Select the percentage of maximum distance at which the interpolation start. A value >= to 1.0 mean brutal fog
    const float fogPercentStart = 0.7;
    float fogStartDistance = fogPercentStart * maxDistance;

    //Calculate fogFactor
    float fogFactor = (colorDistance - fogStartDistance) / (maxDistance - fogStartDistance);
    fogFactor = clamp(fogFactor, 0, 1);
    
    /// Select the good easing function
    // Cosine interpolation
    float alpha = (1.0 - cos(fogFactor * PI)) / 2.0;

    return mix(color, fogColor, alpha);

}

vec3 castRay(vec3 position, vec3 direction, float maxDistance) {
    
    //Parameter background color
    const vec3 skyColor = vec3(0.5294, 0.8078, 0.9216); //Sky

    vec3 color = skyColor;
    // vec3 color = vec3(0.0, 0.0, 0.0); //Black

    //TODO: add distance as parameter of the Shader
    WorldRayCastResult rayCastResult = worldCastRay( position, direction, maxDistance );

    //TODO: Enlever ça mais le garder quelque part, ça fais un mirroir
    // if (rayCastResult.voxel == 4) {

    //     float dotted = dot(direction, rayCastResult.normal);
    //     vec3 newDir = direction + 2.0 * (rayCastResult.normal * dotted);

    //     newDir = -2 * dotted * rayCastResult.normal + direction;

    //     rayCastResult = worldCastRay( rayCastResult.hitPosition, newDir, maxDistance );
    // }

    //TODO: move this to a loop of sun, and select the goo ray casting algorithm
    Ray rayTest = Ray(position, direction);
    Box testBox = Box(sun.position, vec3(0.5), -vec3(0.5), mat3(1.0));

    float distRez = 0;
    vec3 sunNormal = vec3(0.0);
    // bool toucheeed = ourIntersectBoxCommon(testBox, rayTest, distRez, norr, false, false, 1.0/direction);
    RayAABBResult sunRayResult =  box(position, direction, sun.position - 5, sun.position + 5);

    if (sunRayResult.intersect && (rayCastResult.voxel == 0 || sunRayResult.dist < rayCastResult.dist)) {
        color = vec3(1.0);
    }
    else if (rayCastResult.voxel != 0) {

        
        //Hit position informations
        //TODO: better hitdir ?
        vec3 hitDir = -rayCastResult.normal * sign(direction);
        vec3 lightDir = normalize(sun.position - rayCastResult.hitPosition);
        float lightDist = length(sun.position - rayCastResult.hitPosition);
        vec2 uv = computeHitUV (rayCastResult.hitPosition, hitDir, rayCastResult.voxelWorldPosition);
                
        // Ambient occlusion
        float AO = computeAO(rayCastResult.normal, hitDir, rayCastResult.voxelWorldPosition / voxelSize, uv);


        //Calculate shadow ray
        //Note: dist is traveled distance
        float illuminated;
        bool shadow = inShadow( rayCastResult.hitPosition, vec3(0), lightDir, min(lightDist, maxDistance) );
        if (shadow) illuminated = 0.0;
        else illuminated = 1.0;// - (lightDist / maxDistance);

        //Tempo without shadow...
        // float illuminated = 1.0;

        //Fun
        float distFromEdge = max(abs(uv.x - 0.5), abs(uv.y - 0.5)) * 2;
        float scaledTime = time * 3;

        Material material = materials[rayCastResult.voxel];

        // Classic Phong Lighting
        vec3 ambient = computeAmbient(material);
        vec3 diffuse = computeDiffuse(material, rayCastResult.normal, lightDir);
        vec3 specular = computeSpecular(material, normalize(position - rayCastResult.hitPosition), rayCastResult.normal, lightDir);
        
        //With shadow
        vec3 constantColor = ambient;
        vec3 illuminatedColor = (diffuse + specular);

        //TODO: Find the perfect color picking
        color = (constantColor + illuminatedColor * illuminated) * AO;
        // color = (ambient + light.diffuse * illuminated * material.diffuse + specular) * AO;
        // color = material.ambient * AO;


        // if (illuminated == 0.0) {
        //     color = vec3(0.0);
        // }

        if (rayCastResult.voxelWorldPosition == voxelCursorPosition && rayCastResult.normal == faceCursorNormal) {
            if ( distFromEdge >= 0.925 + cos(scaledTime)*0.035 ) {
                color = vec3(1.0);
            }
        }

        //We apply fog on the found color
        color = applyFog(color, skyColor, rayCastResult.dist, maxDistance);

    }
    
    return color;
}
