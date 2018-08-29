#pragma once
#include "defines.h"

#include "renderer_types.h"

int32 createShaderProgram(
	const char *vertexShader,
	const char *controlShader,
	const char *evaluationShader,
	const char *geometryShader,
	const char *fragmentShader,
	const char *computeShader,
	GLuint *program);

int32 getUniform(
	GLuint program,
	char *name,
	UniformType type,
	Uniform *uniform);

int32 setUniform(Uniform uniform, uint32 count, void *data);