#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec2 texCoord0;


// Output Variable (sent down through the Pipeline)
out vec4 outColor;


// Texture
uniform sampler2D texture0;

void main(void) 
{
	outColor = color;
	outColor *= texture(texture0, texCoord0);

}
