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
	fseek(file, 0, SEEK_SET);

	source = malloc(numBytes + 1);
	fread(source, numBytes, 1, file);
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
	printf("Program object value: %d\n", programObject);
	printf("Create Program: %s\n", gluErrorString(glGetError()));

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glAttachShader(programObject, shaders[i]->object);
	}

	glLinkProgram(programObject);
	printf("Link Program: %s\n", gluErrorString(glGetError()));

	glValidateProgram(programObject);
	printf("Validate Program: %s\n", gluErrorString(glGetError()));

	int32 result;
	glGetProgramiv(programObject, GL_VALIDATE_STATUS, &result);
	printf("GL validate status: %d\n", result);

	int32 logSize;
	glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &logSize);
	printf("Program log size: %d\n", logSize);

	char *buffer = malloc(logSize + 1);
	glGetProgramInfoLog(programObject, logSize, 0, buffer);
	printf("Get program info log: %s\n", gluErrorString(glGetError()));
	buffer[logSize] = 0;
	printf("Shader program info log: %s\n", buffer);
	free(buffer);

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glDetachShader(programObject, shaders[i]->object);
	}

	pipeline.object = programObject;
	pipeline.shaders = malloc(numShaders * sizeof(Shader *));
	memcpy(pipeline.shaders, shaders, numShaders * sizeof(Shader *));
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

void unbindShaderPipeline()
{
	glUseProgram(0);
}

Uniform getUniform(ShaderPipeline pipeline, char *name, UniformType type)
{
	Uniform uniform = {};

	uniform.type = type;
	uniform.name = name;

	uniform.location = glGetUniformLocation(pipeline.object, name);

	return uniform;
}

void setUniform(Uniform uniform, void *data)
{
	switch (uniform.type)
	{
	case UNIFORM_MAT4:
	{
		glUniformMatrix4fv(uniform.location, 1, GL_FALSE, data);
	} break;
	case UNIFORM_TEXTURE_2D:
	{
		glUniform1i(uniform.location, *(GLint*)data);
	} break;
	default:
	{

	} break;
	}
}
