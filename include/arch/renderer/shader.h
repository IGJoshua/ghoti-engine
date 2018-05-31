#pragma once
#include "defines.h"

#include "renderer_types.h"

int32 compileShaderFromFile(
	char *filename,
	ShaderType type,
	Shader *shader);
int32 compileShaderFromSource(
	char *source,
	ShaderType type,
	Shader *shader);
int32 composeShaderPipeline(
	Shader **shaders,
	uint32 numShaders,
	ShaderPipeline *pipeline);

void freeShader(Shader shader);
void freeShaderPipeline(ShaderPipeline shader);

void bindShaderPipeline(ShaderPipeline pipeline);
void unbindShaderPipeline();

int32 getUniform(
	ShaderPipeline pipeline,
	char *name,
	UniformType type,
	Uniform *uniform);

int32 setUniform(Uniform uniform, void *data);
