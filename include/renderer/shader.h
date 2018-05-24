#pragma once
#include "defines.h"

#include "renderer_types.h"

Shader compileShaderFromFile(char *filename, ShaderType type);
Shader compileShaderFromSource(char *source, ShaderType type);
ShaderPipeline composeShaderPipeline(Shader **shaders, uint32 numShaders);

void freeShader(Shader shader);
void freeShaderPipeline(ShaderPipeline shader);

void bindShaderPipeline(ShaderPipeline pipeline);
void unbindShaderPipeline();

Uniform getUniform(ShaderPipeline pipeline, char *name, UniformType type);

void setUniform(Uniform uniform, void *data);
