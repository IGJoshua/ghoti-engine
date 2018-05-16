#include "asset_management/asset_manager.h"
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
	Mesh *mesh = &model->mesh;

	bindShaderPipeline(pipeline);

	setUniform(modelUniform, world);
	setUniform(viewUniform, view);
	setUniform(projectionUniform, projection);

	GLint textureIndex = 0;
	setUniform(diffuseTextureUniform, &textureIndex);

	glBindVertexArray(mesh->vertexArray);

	for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

	for (uint32 i = 0; i < model->numMaterials; i++)
	{
		Material *material = &model->materials[i];
		Texture *texture = getTexture(material->diffuseTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);

		uint32 offsetStart = 0;
		if (i > 0)
		{
			offsetStart = model->materials[i - 1].subsetOffset;
		}

		uint32 numIndices = material->subsetOffset;
		if (i > 0)
		{
			numIndices -= model->materials[i - 1].subsetOffset;
		}

		glDrawElements(
			GL_TRIANGLES,
			numIndices,
			GL_UNSIGNED_INT,
			(GLvoid*)(sizeof(uint32) * offsetStart)
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindVertexArray(0);
	glUseProgram(0);

	return 0;
}
