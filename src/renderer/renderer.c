#include "asset_manager/asset_manager.h"
#include "renderer/renderer_types.h"
#include "renderer/shader.h"

Shader vertShader;
Shader fragShader;

ShaderPipeline pipeline;

Uniform modelUniform;
Uniform viewUniform;
Uniform projectionUniform;

Uniform diffuseTextureUniform;
	
int32 initRenderer()
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
}

int32 renderModel(const char *name)
{
	Model *model = getModel(name);
	Mesh *mesh = &model->mesh;

	bindShaderPipeline(pipeline);

	kmMat4 projection;
	kmMat4PerspectiveProjection(&projection, 90, aspect, 0.1f, 1000.0f);
	kmMat4 world;
	kmMat4RotationX(&world, kmDegreesToRadians(-90));
	kmMat4 view;
	kmMat4Translation(&view, 0, 0, 150);
	kmMat4Inverse(&view, &view);

	setUniform(modelUniform, &world);
	setUniform(viewUniform, &view);
	setUniform(projectionUniform, &projection);

	GLint textureIndex = 0;
	setUniform(diffuseTextureUniform, &textureIndex);

	for (uint32 i = model->numMaterials; i++)
	{
		Material *material = &model->materials[i];
		Texture *texture = getTexture(material->diffuseTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);
	}

	glBindVertexArray(mesh->vertexArray);

	for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}
