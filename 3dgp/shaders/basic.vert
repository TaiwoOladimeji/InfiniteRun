#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Uniforms: Material Colours
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform float fogDensity = 0.005;

// Bone Transforms
#define MAX_BONES 100
uniform mat4 bones[MAX_BONES];

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
in ivec4 aBoneId; // Bone Ids
in vec4 aBoneWeight; // Bone Weights


out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out float fogFactor;


// Light declarations
struct AMBIENT
{	
	vec3 color;
};
uniform AMBIENT lightAmbient, lightEmissive;

struct DIRECTIONAL
{	
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir1, lightDir2;

vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	return color;
}

void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate light
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += AmbientLight(lightEmissive);
	color += DirectionalLight(lightDir1);
	color += DirectionalLight(lightDir2);


	// calculate texture coordinate
	texCoord0 = aTexCoord;

	//calculate the fogfactor
	fogFactor = exp2(-fogDensity * length(position));


	mat4 matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] +
				  	   bones[aBoneId[1]] * aBoneWeight[1] +
					bones[aBoneId[2]] * aBoneWeight[2] +
					bones[aBoneId[3]] * aBoneWeight[3]);



}
