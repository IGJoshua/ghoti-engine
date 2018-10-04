#version 420 core

in vec4 fragPosition;

uniform vec3 lightPosition;
uniform float farPlane;

void main()
{
	gl_FragDepth = length(fragPosition.xyz - lightPosition) / farPlane;
}