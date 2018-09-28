#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"

#include "components/component_types.h"
#include "components/camera.h"
#include "components/particle_emitter.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "renderer/renderer_utilities.h"

#define NUM_PARTICLE_VERTEX_ATTRIBUTES 6

typedef struct particle_vertex_t
{
	kmVec3 position;
	kmVec2 size;
	kmVec2 uv;
	kmVec2 spriteSize;
	kmVec4 color;
	int32 texture;
} ParticleVertex;

#define MAX_PARTICLE_COUNT 8192

internal ParticleVertex vertices[MAX_PARTICLE_COUNT];
internal uint32 numVertices;

internal GLuint vertexBuffer;
internal GLuint vertexArray;

#define VERTEX_SHADER_FILE "resources/shaders/particle.vert"
#define GEOMETRY_SHADER_FILE "resources/shaders/particle.geom"
#define FRAGMENT_SHADER_FILE "resources/shaders/particle.frag"

internal GLuint shaderProgram;

internal Uniform viewUniform;
internal Uniform projectionUniform;

#define MAX_PARTICLE_EMITTER_COUNT 128

internal Uniform particleTexturesUniform;
internal uint32 numTextures;

internal uint32 particleRendererSystemRefCount = 0;

internal UUID transformComponentID = {};
internal UUID cameraComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

extern real64 alpha;
extern HashMap particleEmitters;

internal void addVertex(
	kmVec3 *position,
	kmVec2 *size,
	kmVec2 *uv,
	kmVec2 *spriteSize,
	kmVec4 *color,
	int32 texture);
internal void quickSortVertices(
	real32 *cameraDistances,
	int32 left,
	int32 right);
internal void clearVertices(void);

internal void initParticleRendererSystem(Scene *scene)
{
	if (particleRendererSystemRefCount == 0)
	{
		LOG("Initializing particle renderer...\n");

		createShaderProgram(
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			GEOMETRY_SHADER_FILE,
			FRAGMENT_SHADER_FILE,
			NULL,
			&shaderProgram);

		glGenBuffers(1, &vertexBuffer);
		glGenVertexArrays(1, &vertexArray);

		uint32 bufferIndex = 0;

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(ParticleVertex) * MAX_PARTICLE_COUNT,
			NULL,
			GL_DYNAMIC_DRAW);

		glBindVertexArray(vertexArray);
		glVertexAttribPointer(
			bufferIndex++,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, position));

		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, size));

		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, uv));

		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, spriteSize));

		glVertexAttribPointer(
			bufferIndex++,
			4,
			GL_FLOAT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, color));

		glVertexAttribPointer(
			bufferIndex++,
			1,
			GL_INT,
			GL_FALSE,
			sizeof(ParticleVertex),
			(GLvoid*)offsetof(ParticleVertex, texture));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		getUniform(shaderProgram, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);

		getUniform(
			shaderProgram,
			"textures",
			UNIFORM_TEXTURE_2D,
			&particleTexturesUniform);

		clearVertices();

		LOG("Successfully initialized particle renderer\n");
	}

	particleRendererSystemRefCount++;
}

internal void beginParticleRendererSystem(Scene *scene, real64 dt)
{
	if (!particleEmitters)
	{
		return;
	}

	camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID);

	cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		transformComponentID);

	if (!camera || !cameraTransform)
	{
		return;
	}

	clearVertices();

	GLuint textures[MAX_PARTICLE_EMITTER_COUNT];
	memset(textures, 0, MAX_PARTICLE_EMITTER_COUNT * sizeof(GLuint));

	numTextures = 0;

	for (HashMapIterator itr = hashMapGetIterator(particleEmitters);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		ParticleEmitterComponent *particleEmitter =
			*(ParticleEmitterComponent**)hashMapIteratorGetKey(itr);

		if (!particleEmitter->active)
		{
			continue;
		}

		Particle particleTexture = getParticle(
			particleEmitter->currentParticle);

		int32 texture = -1;
		if (strlen(particleTexture.name.string) > 0)
		{
			bool unique = true;
			for (uint32 i = 0; i < numTextures; i++)
			{
				if (textures[i] == particleTexture.id)
				{
					texture = i;
					unique = false;
					break;
				}
			}

			if (unique)
			{
				texture = numTextures;
				textures[numTextures++] = particleTexture.id;
			}
		}
		else
		{
			if (strlen(particleEmitter->currentParticle) > 0)
			{
				continue;
			}
		}

		ParticleList *particleList = hashMapIteratorGetValue(itr);
		for (ListIterator listItr = listGetIterator(&particleList->particles);
			 !listIteratorAtEnd(listItr);
			 listMoveIterator(&listItr))
		{
			ParticleObject *particle = LIST_ITERATOR_GET_ELEMENT(
				ParticleObject,
				listItr);

			if (particle->sprite == -1)
			{
				continue;
			}

			kmVec3 position;
			kmVec3Lerp(
				&position,
				&particle->previousPosition,
				&particle->position,
				alpha);

			addVertex(
				&position,
				&particle->size,
				&particle->uv,
				&particleTexture.spriteSize,
				&particle->color,
				texture);
		}
	}

	if (numVertices == 0)
	{
		return;
	}

	kmVec3 cameraPosition;
	kmVec3Lerp(
		&cameraPosition,
		&cameraTransform->lastGlobalPosition,
		&cameraTransform->globalPosition,
		alpha);

	real32 cameraDistances[MAX_PARTICLE_COUNT];
	for (uint32 i = 0; i < numVertices; i++)
	{
		kmVec3 displacement;
		kmVec3Subtract(
			&displacement,
			&vertices[i].position,
			&cameraPosition);
		cameraDistances[i] = kmVec3LengthSq(&displacement);
	}

	quickSortVertices(cameraDistances, 0, numVertices - 1);

	glUseProgram(shaderProgram);

	cameraSetUniforms(camera, cameraTransform, viewUniform, projectionUniform);

	GLint textureIndex = 0;
	setTextureArrayUniform(
		&particleTexturesUniform,
		numTextures,
		&textureIndex);

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	void *buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(buffer, vertices, numVertices * sizeof(ParticleVertex));
	glUnmapBuffer(GL_ARRAY_BUFFER);

	for (uint8 i = 0; i < NUM_PARTICLE_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	textureIndex = 0;
	activateTextures(numTextures, textures, &textureIndex);

	glDepthMask(GL_FALSE);

	glDrawArrays(GL_POINTS, 0, numVertices);
	logGLError(false, "Failed to draw particle");

	glDepthMask(GL_TRUE);
}

internal void endParticleRendererSystem(Scene *scene, real64 dt)
{
	if (!camera || !cameraTransform)
	{
		return;
	}

	if (!particleEmitters || numVertices == 0)
	{
		return;
	}

	for (uint8 j = 0; j < numTextures; j++)
	{
		glActiveTexture(GL_TEXTURE0 + j);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	for (uint8 j = 0; j < NUM_PARTICLE_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

internal void shutdownParticleRendererSystem(Scene *scene)
{
	if (--particleRendererSystemRefCount == 0)
	{
		LOG("Shutting down particle renderer...\n");

		glBindVertexArray(vertexArray);
		glDeleteBuffers(1, &vertexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &vertexArray);

		glDeleteProgram(shaderProgram);

		LOG("Successfully shut down particle renderer\n");
	}
}

System createParticleRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	cameraComponentID = idFromName("camera");

	system.componentTypes = createList(sizeof(UUID));

	system.init = &initParticleRendererSystem;
	system.begin = &beginParticleRendererSystem;
	system.end = &endParticleRendererSystem;
	system.shutdown = &shutdownParticleRendererSystem;

	return system;
}

void addVertex(
	kmVec3 *position,
	kmVec2 *size,
	kmVec2 *uv,
	kmVec2 *spriteSize,
	kmVec4 *color,
	int32 texture)
{
	if (numVertices + 1 < MAX_PARTICLE_COUNT)
	{
		ParticleVertex *vertex = &vertices[numVertices];
		kmVec3Assign(&vertex->position, position);
		kmVec2Assign(&vertex->size, size);
		kmVec2Assign(&vertex->uv, uv);
		kmVec2Assign(&vertex->spriteSize, spriteSize);
		kmVec4Assign(&vertex->color, color);
		vertex->texture = texture;
		numVertices++;
	}
}

void quickSortVertices(real32 *cameraDistances, int32 left, int32 right)
{
	if (left >= right)
	{
		return;
	}

	real32 pivot = cameraDistances[right];
	int32 count = left;

	for (int32 i = left; i <= right; i++)
	{
		if (cameraDistances[i] >= pivot)
		{
			real32 cameraDistance = cameraDistances[count];
			cameraDistances[count] = cameraDistances[i];
			cameraDistances[i] = cameraDistance;

			ParticleVertex vertex = vertices[count];
			vertices[count] = vertices[i];
			vertices[i] = vertex;

			count++;
		}
	}

	quickSortVertices(cameraDistances, left, count - 2);
	quickSortVertices(cameraDistances, count, right);
}

void clearVertices(void)
{
	memset(vertices, 0, MAX_PARTICLE_COUNT * sizeof(ParticleVertex));
	numVertices = 0;
}