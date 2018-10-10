#version 420 core

const float PI = 3.14159265359;

in vec3 fragPosition;

out vec4 color;

uniform samplerCube cubemapTexture;

void main()
{
	vec3 normal = normalize(fragPosition);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up = cross(normal, right);

	float delta = 0.025;
	uint numSamples = 0;

	vec3 irradiance = vec3(0.0);
	for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += delta)
		{
			vec3 tangentSpacePosition = vec3(
				sin(theta) * cos(phi),
				sin(theta) * sin(phi),
				cos(theta));

			vec3 position =
				tangentSpacePosition.x * right +
				tangentSpacePosition.y * up +
				tangentSpacePosition.z * normal;

			irradiance += texture(cubemapTexture, position).rgb *
				cos(theta) * sin(theta);

			numSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(numSamples));

	color = vec4(irradiance, 1.0);
}