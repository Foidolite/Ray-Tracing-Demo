#version 450

uniform vec2 resolution;
uniform vec2 resInv; //save a division
uniform vec3 eye;
uniform mat3 viewRot;
uniform float heightRatio;

//some helpful macros
#define SAMPLES 2
#define MAX_BOUNCES 25
#define MAX_OBJ 64
#define FLT_MAX 3.402823466e+38
#define EPSILON 0.00005

uniform sampler2DArray texArray;

//hitable object struct
struct Hitable{
	//commented members are positioned to facillitate readability. Their actual declarations are grouped for padding purposes.
	//int type; //-1 = none; 0 = sphere; 1 = plane; 2 = triangle
	//sphere properties
	vec4 center;
	//float radius;
	//plane properties
	vec4 normal;
	vec4 point;
	//triangle properties
	vec4 A;
	vec4 B;
	vec4 C;

	//material properties
	//int matType; //-1 = none; 1 = diffuse; 2 = reflective; 3 = dieletric; 
	vec4 color;
	//reflective properties
	//float fuzz;
	//refractive properties
	//float refIdx;

	//texture properties
	//vec2 uvA;
	//vec2 uvB;
	//vec2 uvC;
	//int texId;

	//group above commented members for padding purposes
	int type; //-1 = none; 0 = sphere; 1 = plane; 2 = triangle
	float radius;
	int matType; //-1 = none; 1 = diffuse; 2 = reflective; 3 = dieletric
	float fuzz;
	float refIdx;
	int texId;
	vec2 uvA;
	vec2 uvB;
	vec2 uvC;

};

layout (std140) uniform worldBlock {
    Hitable world[MAX_OBJ];
};

//returns the two intersection distances, closest positive first. Any negative values should be rejected. ce is the vector from sphere center to eye
vec2 hitSphere(vec3 ce, float r, vec3 ray){
	float a = dot(ray, ray);
	float b = 2*dot(ray, ce);
	float c = dot(ce,ce) - r*r;
	float d = b*b-4*a*c;
	if (d < 0)
		return vec2(-1,-1);
	else{
		d = sqrt(d);
		float invA = 1/(2*a);
		if (-b-d > EPSILON)
			return vec2((-b-d)*invA, (-b+d)*invA);
		else
			return vec2((-b+d)*invA, (-b-d)*invA);
	}
}

//returns the normal given a sphere's center and any point on that sphere
vec3 sphereNormal(vec3 c, vec3 p){
	return normalize(p-c);
}

//returns the intersection distance. n is the normal of the plane. p is a vector from the eye to any point on the plane.
float hitPlane(vec3 n, vec3 p, vec3 ray){
	if (dot(n, ray) != 0)
		return dot(n, p)/dot(n,ray);
	else
		return -1;
}

//generates a pseudo-random number x where 0.0 <= x < 1.0
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//generates a random vector contained in a unit sphere, for lambertian
vec3 randInSphere(vec2 co){
	vec3 p;
	do{
		p = 2*vec3(rand(co),rand(co*1.23),rand(co*1.57)) - vec3(1,1,1);
		co += vec2(1,-1);
	} while (p.x*p.x+p.y*p.y+p.z*p.z > 1);
	return p;
}

//schlick's approximation for reflectance coefficient
float schlick(float cosine,float n1,float n2){
	float r0 = (n1 - n2)/(n1+n2);
	r0 = r0*r0;
	return r0 + (1-r0)*pow(1-cosine,5);
}

//fsaa
vec2 sampleDeflect(vec2 coord, int pNum){
	if (pNum == 0)
		return coord;
	else
		return coord + vec2(sin(pNum*1.618)*0.75,cos(pNum*1.618)*0.75);
}

vec2 sampleDeflectRand(vec2 coord, int pNum){
	if (pNum == 0)
		return coord;
	else
		return coord + vec2(rand(coord*pNum), rand(coord*pNum-1));
}

//main
void main(){
	vec3 finalColor = vec3(0.0);

	for (int i = 0; i < SAMPLES; ++i){
		vec2 adjCoord = sampleDeflectRand(gl_FragCoord.xy - 0.5*resolution,i);
		vec3 viewRay = viewRot*vec3(adjCoord.x*heightRatio, adjCoord.y*heightRatio, -1.0);

		vec3 color = vec3(1.0);
		bool finish = false, escaped = false;
		int bounces = 0;
		vec3 eyePos = eye;

		while (!finish){
			//go through all objects
			int hitIdx = -1;
			float hitPt = FLT_MAX;
			vec3 hitNormal;

			for (int n = 0; n < MAX_OBJ; ++n){
				if (world[n].type == 0){
					vec2 hitPts = hitSphere(eyePos-world[n].center.xyz, world[n].radius, viewRay);
					if (hitPts[0] > EPSILON && hitPts[0] < hitPt){
						hitIdx = n;
						hitPt = hitPts[0];
						hitNormal = sphereNormal(world[n].center.xyz, eyePos+hitPt*viewRay);
					}
				}
				else if (world[n].type == 1){
					float hit = hitPlane(world[n].normal.xyz, world[n].point.xyz - eyePos, viewRay);
					if (hit > EPSILON && hit < hitPt){
						hitIdx = n;
						hitPt = hit;
						hitNormal = world[n].normal.xyz;
					}
				}
				else if (world[n].type == 2){
					vec3 A = world[n].A.xyz, B = world[n].B.xyz, C = world[n].C.xyz;
					vec3 normal = normalize(cross(B-A,C-B));
					float hit = hitPlane(normal, A - eyePos, viewRay);
					if (hit > EPSILON && hit < hitPt){
						vec3 p = eyePos + hit*viewRay;
						if (dot(cross(B-A, p-A),normal) > 0 && dot(cross(C-B, p-B),normal) > 0 && dot(cross(A-C, p-C),normal) > 0){
							hitIdx = n;
							hitPt = hit;
							hitNormal = normal;
						}
					}
				}
			} 

			if (hitIdx != -1){
				if (world[hitIdx].texId != -1 && world[hitIdx].type == 2){
					vec3 coeff = eyePos+viewRay*hitPt;
					mat3 solve = inverse(mat3(world[hitIdx].A.xyz,world[hitIdx].B.xyz,world[hitIdx].C.xyz));
					coeff = solve*coeff;
					mat3x2 uvMat = mat3x2(world[hitIdx].uvA, world[hitIdx].uvB, world[hitIdx].uvC);
					vec2 uv = uvMat*coeff;

					color *= texture(texArray, vec3(uv,world[hitIdx].texId));
				}					
				else
					color *= world[hitIdx].color.rgb;

				if (world[hitIdx].matType == 1){
					eyePos = eyePos+viewRay*hitPt;
					viewRay = hitNormal + randInSphere(adjCoord.xy+bounces);
				}
				else if (world[hitIdx].matType == 2){
					eyePos = eyePos+viewRay*hitPt;
					viewRay = viewRay - 2*dot(hitNormal,viewRay)*hitNormal + world[hitIdx].fuzz*randInSphere(adjCoord.xy+bounces);
				}
				else if (world[hitIdx].matType == 3){
					eyePos = eyePos+viewRay*hitPt;

					float nRatio;
					float c = dot(hitNormal, viewRay);
					if (c < 0){ //entry
						c = -c; 
						hitNormal = -hitNormal; 
						nRatio = 1.0/world[hitIdx].refIdx;
					}
					else //exit
						nRatio = world[hitIdx].refIdx;

					float d = 1-nRatio*nRatio*(1-c*c);
					if (d > 0){
						if (rand(adjCoord.xy + bounces) > schlick(abs(dot(hitNormal, normalize(viewRay))), 1.0, world[hitIdx].refIdx)){
							d = sqrt(d);
							viewRay = d*hitNormal + nRatio*(viewRay - c*hitNormal);
						}
						else
							viewRay = viewRay - 2*dot(viewRay,hitNormal)*hitNormal;
					}
					else
						viewRay = viewRay - 2*dot(viewRay,hitNormal)*hitNormal;
				}
			}
			else{
				escaped = true;
				finish = true;
			}

			++bounces;
			if (bounces == MAX_BOUNCES)
				finish = true;
		}

		if (escaped){
			vec3 unitRay = viewRay/dot(viewRay,viewRay);
			color *= (1-unitRay.y)*vec3(1.0,1.0,1.0) + (unitRay.y)*vec3(0.5,0.7,1.0); //multiply by sky color
		}

		finalColor += color;
	}

	float gamma = 2.2;
	gl_FragColor = vec4(pow(finalColor/SAMPLES, vec3(1/gamma)),1.0);
}