#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;



in vec3 FragPosG[];
in vec3 NormalG[];
in vec3 ColorsG[];

in vec3 verti[];


out vec3 FragPos;
out vec3 Normal;
out vec3 Colors;



void sameTriangle(int index){

	for(int i = 0; i < 3; ++i){

		FragPos = FragPosG[index*3 + i];
		//Normal = NormalG[index*3 + i];
		Colors = ColorsG[index*3 + i];

		gl_Position = gl_in[index*3 + i].gl_Position;

		//colCol = vec3(verti[i].x, verti[i].z, 0);

		EmitVertex();
	}
	
	EndPrimitive();
}

vec3 calculNormal(){
	vec3 v1 = vec3(FragPosG[1].x - FragPosG[0].x,
					FragPosG[1].y - FragPosG[0].y,
					FragPosG[1].z - FragPosG[0].z);

	vec3 v2 = vec3(FragPosG[2].x - FragPosG[0].x,
					FragPosG[2].y - FragPosG[0].y,
					FragPosG[2].z - FragPosG[0].z);

	vec3 n = vec3(0, -1, 0);

	//colCol = vec3(0, 0, 0);

	if(FragPosG[2].x >= FragPosG[1].x){
		n = vec3(v1.y*v2.z - v1.z*v2.y,
				 v1.z*v2.x - v1.x*v2.z,
				 v1.x*v2.y - v1.y*v2.x);

		//colCol = vec3(1, 1, 1);

		//n = vec3(0, 1, 0);
	}
	else{
		n = vec3(v2.y*v1.z - v2.z*v1.y,
				v2.z*v1.x - v2.x*v1.z,
				v2.x*v1.y - v2.y*v1.x);
	}

	return normalize(n);
}

void main(){

	//Normal = cross(FragPosG[1].xyz - FragPosG[0].xyz, FragPosG[2].xyz - FragPosG[0].xyz);

	Normal = calculNormal();

    sameTriangle(0); // first vertex normal

}