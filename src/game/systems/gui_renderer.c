#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

#include <GL/glew.h>

#include <kazmath/mat4.h>

internal uint32 guiRendererRefCount = 0;

extern struct nk_context ctx;
extern struct nk_buffer cmds;

extern int32 viewportWidth;
extern int32 viewportHeight;

#define NUM_GUI_VERTEX_ATTRIBUTES 3

extern GLuint vertexBuffer;
extern GLuint vertexArray;
extern GLuint indexBuffer;

internal GLuint shaderProgram;

internal Uniform projectionUniform;
internal Uniform fontUniform;

internal GLboolean glBlendValue;
internal GLint glBlendEquationValue;
internal GLint glSrcBlendFuncValue;
internal GLint glDstBlendFuncValue;
internal GLboolean glCullFaceValue;
internal GLboolean glDepthTestValue;
internal GLboolean glScissorTestValue;

internal void initGUIRendererSystem(Scene *scene)
{
	if (guiRendererRefCount == 0)
	{
		LOG("Initializing GUI renderer...\n");

		createShaderProgram(
			"resources/shaders/gui.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/gui.frag",
			NULL,
			&shaderProgram);

		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);

		getUniform(
			shaderProgram,
			"font",
			UNIFORM_TEXTURE_2D,
			&fontUniform);

		LOG("Successfully initialized GUI renderer\n");
	}

	guiRendererRefCount++;
}

internal void beginGUIRendererSystem(Scene *scene, real64 dt)
{
	kmMat4 projectionMatrix;
	kmMat4OrthographicProjection(
		&projectionMatrix,
		0.0f,
		viewportWidth,
		viewportHeight,
		0.0f,
		0.0f,
		2.0f);

	glUseProgram(shaderProgram);

	setUniform(projectionUniform, 1, &projectionMatrix);

	glActiveTexture(GL_TEXTURE0);

	GLint textureIndex = 0;
	setUniform(fontUniform, 1, &textureIndex);

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	for (uint8 i = 0; i < NUM_GUI_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glGetBooleanv(GL_BLEND, &glBlendValue);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &glBlendEquationValue);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &glSrcBlendFuncValue);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &glDstBlendFuncValue);
	glGetBooleanv(GL_CULL_FACE, &glCullFaceValue);
	glGetBooleanv(GL_DEPTH_TEST, &glDepthTestValue);
	glGetBooleanv(GL_SCISSOR_TEST, &glScissorTestValue);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	const struct nk_draw_command *cmd;
	const nk_draw_index *offset = NULL;

	nk_draw_foreach(cmd, &ctx, &cmds)
	{
		if (!cmd->elem_count)
		{
			continue;
		}

		glBindTexture(GL_TEXTURE_2D, cmd->texture.id);

		glScissor(
			cmd->clip_rect.x,
			viewportHeight - (cmd->clip_rect.y + cmd->clip_rect.h),
			cmd->clip_rect.w,
			cmd->clip_rect.h);

		glDrawElements(
			GL_TRIANGLES,
			cmd->elem_count,
			GL_UNSIGNED_SHORT,
			offset);

		logGLError(false, "Error when drawing GUI");

		offset += cmd->elem_count;
	}
}

internal void endGUIRendererSystem(Scene *scene, real64 dt)
{
	glBlendValue ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	glBlendEquation(glBlendEquationValue);
	glBlendFunc(glSrcBlendFuncValue, glDstBlendFuncValue);
	glCullFaceValue ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	glDepthTestValue ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	glScissorTestValue ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);

	for (uint8 i = 0; i < NUM_GUI_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

internal void shutdownGUIRendererSystem(Scene *scene)
{
	if (--guiRendererRefCount == 0)
	{
		LOG("Shutting down GUI renderer...\n");

		glDeleteProgram(shaderProgram);

		LOG("Successfully shut down GUI renderer\n");
	}
}

System createGUIRendererSystem(void)
{
	System system = {};

	system.componentTypes = createList(sizeof(UUID));

	system.init = &initGUIRendererSystem;
	system.begin = &beginGUIRendererSystem;
	system.end = &endGUIRendererSystem;
	system.shutdown = &shutdownGUIRendererSystem;

	return system;
}