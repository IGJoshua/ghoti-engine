#include "renderer/shader.h"

#include "core/log.h"

#include "file/utilities.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

int32 compileShaderFromFile(char *filename, ShaderType type, Shader *shader)
{
	uint64 fileLength;
	char *source = readFile(filename, &fileLength);
	if (source)
	{
		LOG("Loaded %s\n", filename);
		return compileShaderFromSource(source, type, shader);
	}

	return -1;
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
	LOG("Compilation of Shader: %s\n", gluErrorString(glError));
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
	LOG("Shader Pipeline Creation: %s\n", gluErrorString(glError));
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
	LOG("Shader Pipeline Linking: %s\n", gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	glValidateProgram(programObject);

	glError = glGetError();
	LOG("Shader Pipeline Validation: %s\n", gluErrorString(glError));
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

int32 getUniform(
	ShaderPipeline pipeline,
	char *name,
	UniformType type,
	Uniform *uniform)
{
	uniform->type = type;
	uniform->name = name;

	uniform->location = glGetUniformLocation(pipeline.object, name);
	GLenum glError = glGetError();
	LOG("Get Uniform (%s): %s\n", name, gluErrorString(glError));
	if (glError != GL_NO_ERROR)
	{
		return -1;
	}

	return 0;
}

int32 setUniform(Uniform uniform, uint32 count, void *data)
{
	if (uniform.location > -1)
	{
		GLint boolData;
		switch (uniform.type)
		{
			case UNIFORM_MAT4:
				glUniformMatrix4fv(uniform.location, count, GL_FALSE, data);
				break;
			case UNIFORM_VEC3:
				glUniform3fv(uniform.location, count, data);
				break;
			case UNIFORM_BOOL:
				boolData = *(bool*)data ? true : false;
				glUniform1iv(uniform.location, count, &boolData);
				break;
			case UNIFORM_TEXTURE_2D:
				glUniform1iv(uniform.location, count, data);
				break;
			default:
				break;
		}

		GLenum glError = glGetError();
		if (glError != GL_NO_ERROR)
		{
			LOG("Set Uniform (%s): %s\n",
				uniform.name,
				gluErrorString(glError));
			return -1;
		}
	}

	return 0;
}
