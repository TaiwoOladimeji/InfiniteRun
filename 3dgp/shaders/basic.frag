#version 330

// Input Variables (received from Vertex Shader)
in vec4 color;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;


void main(void) 
{
	outColor = color;
}
