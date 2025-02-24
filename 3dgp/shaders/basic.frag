#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in float fogFactor;


// Output Variable (sent down through the Pipeline)
out vec4 outColor;


uniform sampler2D texture0;			// Texture
uniform vec3 materialSpecular;
uniform vec3 materialDiffuse;
uniform float materialShininess;
uniform mat4 matrixView;

uniform vec3 fogColour = vec3(0.96, 0.96, 0.95);			//Fog


struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform int lightAttOn = 0;
uniform POINT lightPoint1, lightPoint2, lightPoint3, lightPoint4, lightPoint5;

vec4 compPoint(vec3 materialDiffuse, vec3 materialSpecular, float materialShininess, POINT light)
{
	vec4 result = vec4(0, 0, 0, 1);

	// diffuse
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(L, normal.xyz);
	result += vec4(light.diffuse * materialDiffuse, 1) * max(NdotL, 0);

	// specular
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal.xyz);
	float RdotV = dot(R, V);
	if (RdotV > 0)
		result += vec4(light.specular * materialSpecular * pow(max(RdotV, 0), materialShininess), 1);

	return result;
}

vec4 compPointAtt(vec3 materialDiffuse, vec3 materialSpecular, float materialShininess, POINT light)
{
	vec4 result = vec4(0, 0, 0, 1);

	// diffuse
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(L, normal.xyz);
	result += vec4(light.diffuse * materialDiffuse, 1) * max(NdotL, 0);

	// specular
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal.xyz);
	float RdotV = dot(R, V);
	if (RdotV > 0)
		result += vec4(light.specular * materialSpecular * pow(max(RdotV, 0), materialShininess), 1);

	// attenuation
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (dist * dist) / 0.25;

	return result * att;
}


void main(void) 
{
	outColor = color;
	outColor *= texture(texture0, texCoord0);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
	if (lightAttOn == 1)
	{

		outColor += compPointAtt(materialDiffuse, materialSpecular, materialShininess, lightPoint1);
		outColor += compPointAtt(materialDiffuse, materialSpecular, materialShininess, lightPoint2);
		outColor += compPointAtt(materialDiffuse, materialSpecular, materialShininess, lightPoint3);
		outColor += compPointAtt(materialDiffuse, materialSpecular, materialShininess, lightPoint4);
		outColor += compPointAtt(materialDiffuse, materialSpecular, materialShininess, lightPoint5);
	}
}
