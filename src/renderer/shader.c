#include "renderer/shader.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHUNKS 128
#define CHUNK_SIZE 1024

Shader compileShaderFromFile(char *filename, ShaderType type)
{
	char *source = 0;

	FILE *file = fopen(filename, "r");

	fseek(file, 0, SEEK_END);
	uint64 numBytes = ftell(file);

	source = malloc(numBytes + 1);
	fgets(source, numBytes, file);
	source[numBytes] = 0;

	fclose(file);

	return compileShaderFromSource(source, type);
}

internal inline
GLenum shaderType(ShaderType type)
{
	return type == SHADER_VERTEX
		? GL_VERTEX_SHADER
		: type == SHADER_GEOMETRY
		? GL_GEOMETRY_SHADER
		: type == SHADER_CONTROL
		? GL_TESS_CONTROL_SHADER
		: type == SHADER_EVALUATION
		? GL_TESS_EVALUATION_SHADER
		: type == SHADER_COMPUTE
		? GL_COMPUTE_SHADER
		: type == SHADER_FRAGMENT
		? GL_FRAGMENT_SHADER
		: GL_INVALID_ENUM;
}

Shader compileShaderFromSource(char *source, ShaderType type)
{
	Shader shader = {};

	GLuint shaderObject = glCreateShader(shaderType(type));

	glShaderSource(shaderObject, 1, (const GLchar * const *)&source, 0);

	glCompileShader(shaderObject);

	shader.object = shaderObject;
	shader.source = source;

	return shader;
}

ShaderPipeline composeShaderPipeline(Shader **shaders, uint32 numShaders)
{
	ShaderPipeline pipeline = {};

	GLuint programObject = glCreateProgram();

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glAttachShader(programObject, shaders[i]->object);
	}

	glLinkProgram(programObject);

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glDetachShader(programObject, shaders[i]->object);
	}

	pipeline.shaders = malloc(numShaders * sizeof(Shader *));
	memcpy(pipeline.shaders, shaders, numShaders * sizeof(Shader *));

	pipeline.object = programObject;
	pipeline.shaderCount = numShaders;

	return pipeline;
}

void freeShader(Shader shader)
{
	glDeleteShader(shader.object);
	free(shader.source);
	shader.type = SHADER_INVALID;
}

void freeShaderPipeline(ShaderPipeline pipeline)
{
	glDeleteProgram(pipeline.object);
	free(pipeline.shaders);
	pipeline.shaderCount = 0;
}

void bindShaderPipeline(ShaderPipeline pipeline)
{
	glUseProgram(pipeline.object);
}
