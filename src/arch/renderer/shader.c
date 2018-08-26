#include "renderer/shader.h"

#include "core/log.h"

#include "file/utilities.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

internal int32 compileShaderFromFile(
	const char *filename,
	ShaderType type,
	Shader *shader);
internal int32 compileShaderFromSource(
	char *source,
	ShaderType type,
	Shader *shader);

internal void freeShader(Shader shader);

int32 compileShaderFromFile(
	const char *filename,
	ShaderType type,
	Shader *shader)
{
	shader->source = NULL;

	if (!filename)
	{
		return 0;
	}

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

void freeShader(Shader shader)
{
	if (shader.source)
	{
		glDeleteShader(shader.object);
		free(shader.source);
		shader.type = SHADER_INVALID;
	}
}

int32 createShaderProgram(
	const char *vertexShader,
	const char *controlShader,
	const char *evaluationShader,
	const char *geometryShader,
	const char *fragmentShader,
	const char *computeShader,
	GLuint *program)
{
	int32 error = 0;

	const char *shaderFiles[SHADER_TYPE_COUNT] =
	{
		vertexShader,
		controlShader,
		evaluationShader,
		geometryShader,
		fragmentShader,
		computeShader
	};

	Shader shaders[SHADER_TYPE_COUNT];

	for (uint8 i = 0; i < SHADER_TYPE_COUNT; i++)
	{
		error = compileShaderFromFile(
			shaderFiles[i],
			(ShaderType)i,
			&shaders[i]);
		if (error == -1)
		{
			break;
		}
	}

	if (error != -1)
	{
		*program = glCreateProgram();

		GLenum glError = glGetError();
		LOG("Shader Program Creation: %s\n", gluErrorString(glError));
		if (glError != GL_NO_ERROR)
		{
			error = -1;
		}

		if (error != -1)
		{
			for (uint8 i = 0; i < SHADER_TYPE_COUNT; i++)
			{
				if (shaders[i].source)
				{
					glAttachShader(*program, shaders[i].object);
				}
			}

			glLinkProgram(*program);

			glError = glGetError();
			LOG("Shader Program Linking: %s\n", gluErrorString(glError));
			if (glError != GL_NO_ERROR)
			{
				error = -1;
			}

			if (error != -1)
			{
				glValidateProgram(*program);

				glError = glGetError();
				LOG("Shader Program Validation: %s\n", gluErrorString(glError));
				if (glError != GL_NO_ERROR)
				{
					return -1;
				}
			}
		}
	}

	for (uint8 i = 0; i < SHADER_TYPE_COUNT; i++)
	{
		freeShader(shaders[i]);
	}

	return error;
}

int32 getUniform(
	GLuint program,
	char *name,
	UniformType type,
	Uniform *uniform)
{
	uniform->type = type;
	uniform->name = name;

	uniform->location = glGetUniformLocation(program, name);
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
