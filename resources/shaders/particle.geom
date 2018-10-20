#version 420 core

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in vec2 geomSize[];
in vec2 geomUV[];
in vec2 geomSpriteSize[];
in vec4 geomColor[];
in int geomTexture[];

out vec2 fragUV;
out vec4 fragColor;
flat out int fragTexture;

uniform mat4 projection;

void main()
{
	vec4 position = gl_in[0].gl_Position;
	vec2 halfSize = geomSize[0] / 2;

	// Bottom Left

	fragUV = vec2(
		geomUV[0].x,
		clamp(geomUV[0].y + geomSpriteSize[0].y, 0.0, 1.0));
	fragColor = geomColor[0];
	fragTexture = geomTexture[0];

	vec2 corner = position.xy + vec2(-halfSize.x, -halfSize.y);
	gl_Position = projection * vec4(corner, position.zw);

	EmitVertex();

	// Bottom Right

	fragUV = vec2(
		clamp(geomUV[0].x + geomSpriteSize[0].x, 0.0, 1.0),
		clamp(geomUV[0].y + geomSpriteSize[0].y, 0.0, 1.0));
	fragColor = geomColor[0];
	fragTexture = geomTexture[0];

	corner = position.xy + vec2(halfSize.x, -halfSize.y);
	gl_Position = projection * vec4(corner, position.zw);

	EmitVertex();

	// Top Left

	fragUV = vec2(geomUV[0].x, geomUV[0].y);
	fragColor = geomColor[0];
	fragTexture = geomTexture[0];

	corner = position.xy + vec2(-halfSize.x, halfSize.y);
	gl_Position = projection * vec4(corner, position.zw);

	EmitVertex();

	// Top Right

	fragUV = vec2(
		clamp(geomUV[0].x + geomSpriteSize[0].x, 0.0, 1.0),
		geomUV[0].y);
	fragColor = geomColor[0];
	fragTexture = geomTexture[0];

	corner = position.xy + vec2(halfSize.x, halfSize.y);
	gl_Position = projection * vec4(corner, position.zw);

	EmitVertex();

	EndPrimitive();
}