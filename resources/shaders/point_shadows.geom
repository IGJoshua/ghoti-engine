#version 420 core

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

out vec4 fragPosition;

uniform mat4 lightTransforms[6];

void main()
{
	for (int i = 0; i < 6; i++)
	{
		gl_Layer = i;
		for (uint j = 0; j < 3; j++)
		{
			fragPosition = gl_in[j].gl_Position;
			gl_Position = lightTransforms[i] * fragPosition;
			EmitVertex();
		}

		EndPrimitive();
	}
}