
//Note: UV are like the texture position of the hitted voxel face
vec2 computeHitUV(vec3 hitPos, vec3 hitDir, vec3 hitVoxelChunkPos) {
    vec3 uvw = (hitPos - (hitVoxelChunkPos + voxelSize * 0.5)) / voxelSize;
    return vec2( dot(hitDir.yzx, uvw), dot(hitDir.zxy, uvw) ) + 0.5;
}

//TODO: rassembler tout les trucs en rapport avec l'AO dans un même frag file
ivec4 edges( in vec3 vos, in vec3 nor, in vec3 dir ) {
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

ivec4 corners( in vec3 vos, in vec3 nor, in vec3 dir ) {
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

//TODO: verify impact on performance of AO
float computeAO(vec3 norm, vec3 hitDir, vec3 hitVoxelChunkPos, vec2 uv) {

    //TODO: voir si utile
    roundPosition(hitVoxelChunkPos);

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
