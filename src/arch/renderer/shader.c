#include "renderer/shader.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHUNKS 128
#define CHUNK_SIZE 1024

int32 compileShaderFromFile(char *filename, ShaderType type, Shader *shader)
{
	char *source = 0;

	FILE *file = fopen(filename, "r");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		uint64 numBytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		source = malloc(numBytes + 1);
		fread(source, numBytes, 1, file);
		source[numBytes] = 0;

		fclose(file);

		printf("Loaded %s.\n", filename);
	}
	else
	{
		printf("Unable to open %s.\n", filename);
		return -1;
	}

	return compileShaderFromSource(source, type, shader);
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

int32 compileShaderFromSource(char *source, ShaderType type, Shader *shader)
{
	GLuint shaderObject = glCreateShader(shaderType(type));

	glShaderSource(shaderObject, 1, (const GLchar * const *)&source, 0);

	glCompileShader(shaderObject);
	GLenum glError = glGetError();
	printf("Compilation of Shader: %s\n", gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	shader->object = shaderObject;
	shader->source = source;

	return 0;
}

int32 composeShaderPipeline(Shader **shaders, uint32 numShaders, ShaderPipeline *pipeline)
{
	GLuint programObject = glCreateProgram();

	GLenum glError = glGetError();
	printf("Shader Pipeline Creation: %s\n", gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glAttachShader(programObject, shaders[i]->object);
	}

	glLinkProgram(programObject);

	glError = glGetError();
	printf("Shader Pipeline Linking: %s\n", gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	glValidateProgram(programObject);

	glError = glGetError();
	printf("Shader Pipeline Validation: %s\n", gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	for (uint32 i = 0; i < numShaders; ++i)
	{
		glDetachShader(programObject, shaders[i]->object);
	}

	pipeline->object = programObject;
	pipeline->shaders = malloc(numShaders * sizeof(Shader *));
	memcpy(pipeline->shaders, shaders, numShaders * sizeof(Shader *));
	pipeline->shaderCount = numShaders;

	return 0;
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

void unbindShaderPipeline(void)
{
	glUseProgram(0);
}

int32 getUniform(ShaderPipeline pipeline, char *name, UniformType type, Uniform *uniform)
{
	uniform->type = type;
	uniform->name = name;

	uniform->location = glGetUniformLocation(pipeline.object, name);
	GLenum glError = glGetError();
	printf("Get Uniform (%s): %s\n", name, gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	return 0;
}

int32 setUniform(Uniform uniform, void *data)
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

		GLenum glError = glGetError();
		printf("Set Uniform (%s): %s\n", uniform.name, gluErrorString(glError));
		if (glError != GL_NO_ERROR)
		{
			return -1;
		}
	}

	return 0;
}
