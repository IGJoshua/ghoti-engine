#include "asset_management/model.h"
#include "asset_management/texture.h"
#include "renderer/renderer_types.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

#include <GLFW/glfw3.h>

#include <malloc.h>

Shader vertShader;
Shader fragShader;

ShaderPipeline pipeline;

Uniform modelUniform;
Uniform viewUniform;
Uniform projectionUniform;

Uniform diffuseTextureUniform;

int32 initRenderer(void)
{
	vertShader = compileShaderFromFile("resources/shaders/base.vert", SHADER_VERTEX);
	fragShader = compileShaderFromFile("resources/shaders/color.frag", SHADER_FRAGMENT);

	{
		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		pipeline = composeShaderPipeline(program, 2);
	}

	freeShader(vertShader);
	freeShader(fragShader);
	free(pipeline.shaders);
	pipeline.shaderCount = 0;

	modelUniform = getUniform(pipeline, "model", UNIFORM_MAT4);
	viewUniform = getUniform(pipeline, "view", UNIFORM_MAT4);
	projectionUniform = getUniform(pipeline, "projection", UNIFORM_MAT4);

	diffuseTextureUniform = getUniform(pipeline, "diffuseTexture", UNIFORM_TEXTURE_2D);

	return 0;
}

int32 renderModel(const char *name, kmMat4 *world, kmMat4 *view, kmMat4 *projection)
{
	Model *model = getModel(name);

	bindShaderPipeline(pipeline);

	setUniform(modelUniform, world);
	setUniform(viewUniform, view);
	setUniform(projectionUniform, projection);

	GLint textureIndex = 0;
	setUniform(diffuseTextureUniform, &textureIndex);

	for (uint32 i = 0; i < model->numMeshes; i++)
	{
		Mesh *mesh = &model->meshes[i];

		glBindVertexArray(mesh->vertexArray);

		for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
		{
			glEnableVertexAttribArray(i);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		Material *material = &model->materials[i];
		Texture *texture = getTexture(material->diffuseTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
		{
			glDisableVertexAttribArray(i);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glUseProgram(0);

	return 0;
}
