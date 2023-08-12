
float maxComponent (vec3 v) {
    return max (max (v.x, v.y), v.z);
}

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

struct RayAABBResult {
    bool intersect;
    vec3 hitPosition;
    vec3 normal;
    float dist;
};

RayAABBResult box(vec3 ray_origin, vec3 ray_dir, vec3 minPos, vec3 maxPos) {

    RayAABBResult result;

    vec3 inverse_dir = 1.0 / ray_dir;
    vec3 tbot = inverse_dir * (minPos - ray_origin);
    vec3 ttop = inverse_dir * (maxPos - ray_origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 traverse = max(tmin.xx, tmin.yz);
    float traverselow = max(traverse.x, traverse.y);
    traverse = min(tmax.xx, tmax.yz);
    float traversehi = min(traverse.x, traverse.y);

    result.intersect = traversehi > max(traverselow, 0.0);

    if (result.intersect) {

        result.dist = traverselow;
        result.hitPosition = ray_origin + ray_dir * result.dist;

        vec3 c = (minPos + maxPos) * 0.5;
        vec3 p = result.hitPosition - c;
        vec3 d = (minPos - maxPos) * 0.5;
        
        float bias = 1.001;

        result.normal = normalize(vec3(
            float(int(p.x / abs(d.x) * bias)),
            float(int(p.y / abs(d.y) * bias)),
            float(int(p.z / abs(d.z) * bias))
        ));

        result.hitPosition += result.normal * 0.00001;

    }

    return result;

}
