#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/font.h"

#include "components/component_types.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

#include <GL/glew.h>

internal UUID guiTransformComponentID = {};
internal UUID panelComponentID = {};
internal UUID widgetListComponentID = {};
internal UUID textComponentID = {};
internal UUID buttonComponentID = {};

uint32 guiRefCount = 0;

struct nk_context ctx;
struct nk_buffer cmds;

#define DEFAULT_FONT "default_font"
#define DEFAULT_FONT_SIZE 18

internal Font *defaultFont;

internal uint32 viewportWidth;
internal uint32 viewportHeight;

internal struct nk_convert_config config;

#define MAX_GUI_VERTEX_COUNT 512 * 2048
#define MAX_GUI_INDEX_COUNT 128 * 2048

#define NUM_GUI_VERTEX_ATTRIBUTES 3

typedef struct gui_vertex_t {
	kmVec2 position;
	kmVec2 uv;
	kmVec4 color;
} GUIVertex;

const struct nk_draw_vertex_layout_element vertex_layout[] = {
	{
		NK_VERTEX_POSITION,
		NK_FORMAT_FLOAT,
		NK_OFFSETOF(GUIVertex, position)
	},

	{
		NK_VERTEX_TEXCOORD,
		NK_FORMAT_FLOAT,
		NK_OFFSETOF(GUIVertex, uv)
	},

	{
		NK_VERTEX_COLOR,
		NK_FORMAT_R32G32B32A32_FLOAT,
		NK_OFFSETOF(GUIVertex, color)
	},

	{ NK_VERTEX_LAYOUT_END }
};

GLuint vertexBuffer;
GLuint vertexArray;
GLuint indexBuffer;

internal struct nk_color getColor(kmVec4 *color);
internal struct nk_rect getRect(
	GUITransformComponent *guiTransform,
	real32 width,
	real32 height);

internal void addWidgets(
	Scene *scene,
	UUID entity,
	real32 panelWidth,
	real32 panelHeight);
internal void addText(Scene *scene, UUID entity);
internal void addButton(Scene *scene, UUID entity);

internal void initGUISystem(Scene *scene)
{
	if (guiRefCount == 0)
	{
		if (!nk_init_default(&ctx, NULL))
		{
			LOG("Failed to initialize the GUI\n");
		}
		else
		{
			nk_buffer_init_default(&cmds);

			loadFont(DEFAULT_FONT, DEFAULT_FONT_SIZE);
			defaultFont = getFont(DEFAULT_FONT, DEFAULT_FONT_SIZE);
			nk_style_set_font(&ctx, &defaultFont->font->handle);

			memset(&config, 0, sizeof(struct nk_convert_config));
			config.shape_AA = NK_ANTI_ALIASING_ON;
			config.line_AA = NK_ANTI_ALIASING_ON;
			config.vertex_layout = vertex_layout;
			config.vertex_size = sizeof(GUIVertex);
			config.vertex_alignment = NK_ALIGNOF(GUIVertex);
			config.circle_segment_count = 22;
			config.curve_segment_count = 22;
			config.arc_segment_count = 22;
			config.global_alpha = 1.0f;
			config.null = defaultFont->null;

			glGenBuffers(1, &vertexBuffer);
			glGenVertexArrays(1, &vertexArray);

			uint32 bufferIndex = 0;

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBindVertexArray(vertexArray);

			glVertexAttribPointer(
				bufferIndex++,
				2,
				GL_FLOAT,
				GL_FALSE,
				sizeof(GUIVertex),
				(GLvoid*)offsetof(GUIVertex, position));
			glVertexAttribPointer(
				bufferIndex++,
				2,
				GL_FLOAT,
				GL_FALSE,
				sizeof(GUIVertex),
				(GLvoid*)offsetof(GUIVertex, uv));
			glVertexAttribPointer(
				bufferIndex++,
				4,
				GL_FLOAT,
				GL_FALSE,
				sizeof(GUIVertex),
				(GLvoid*)offsetof(GUIVertex, color));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			glGenBuffers(1, &indexBuffer);
		}
	}

	guiRefCount++;
}

internal void beginGUISystem(Scene *scene, real64 dt)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	viewportWidth = viewport[2];
	viewportHeight = viewport[3];

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(GUIVertex) * MAX_GUI_VERTEX_COUNT,
		NULL,
		GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16) * MAX_GUI_INDEX_COUNT,
		NULL,
		GL_STREAM_DRAW);
}

internal void runGUISystem(Scene *scene, UUID entityID, real64 dt)
{
	PanelComponent *panel = sceneGetComponentFromEntity(
		scene,
		entityID,
		panelComponentID);

	if (!panel->enabled)
	{
		return;
	}

	ctx.style.window.fixed_background = nk_style_item_color(
		getColor(&panel->color));

	Font *font = getFont(panel->font, panel->fontSize);
	if (!font)
	{
		font = defaultFont;
	}

	// TODO: Set before each draw call or here?
	nk_style_set_font(&ctx, &font->font->handle);
	config.null = font->null;

	GUITransformComponent *guiTransform = sceneGetComponentFromEntity(
		scene,
		entityID,
		guiTransformComponentID);

	struct nk_rect rect = getRect(guiTransform, viewportWidth, viewportHeight);

	if (nk_begin(
		&ctx,
		entityID.string,
		rect,
		NK_WINDOW_NO_INPUT | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_space_begin(&ctx, NK_STATIC, 0, INT_MAX);
		addWidgets(scene, panel->widgetList, rect.w, rect.h);
		nk_layout_space_end(&ctx);
	}

	nk_end(&ctx);

	void *vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	void *indices = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	struct nk_buffer vertexBufferData;
	nk_buffer_init_fixed(
		&vertexBufferData,
		vertices,
		sizeof(GUIVertex) * MAX_GUI_VERTEX_COUNT);

	struct nk_buffer indexBufferData;
	nk_buffer_init_fixed(
		&indexBufferData,
		indices,
		sizeof(uint16) * MAX_GUI_INDEX_COUNT);

	nk_convert(&ctx, &cmds, &vertexBufferData, &indexBufferData, &config);

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	nk_buffer_free(&vertexBufferData);
	nk_buffer_free(&indexBufferData);
}

internal void endGUISystem(Scene *scene, real64 dt)
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	nk_clear(&ctx);
}

internal void shutdownGUISystem(Scene *scene)
{
	if (--guiRefCount == 0)
	{
		nk_free(&ctx);
		nk_buffer_free(&cmds);

		glBindVertexArray(vertexArray);
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &vertexArray);
	}
}

System createGUISystem(void)
{
	guiTransformComponentID = idFromName("gui_transform");
	panelComponentID = idFromName("panel");
	widgetListComponentID = idFromName("widget_list");
	textComponentID = idFromName("text");
	buttonComponentID = idFromName("button");

	System system = {};

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &guiTransformComponentID);
	listPushFront(&system.componentTypes, &panelComponentID);

	system.init = &initGUISystem;
	system.begin = &beginGUISystem;
	system.run = &runGUISystem;
	system.end = &endGUISystem;
	system.shutdown = &shutdownGUISystem;

	return system;
}

struct nk_color getColor(kmVec4 *color)
{
	return nk_rgba(
		color->x * 255,
		color->y * 255,
		color->z * 255,
		color->w * 255);
}

struct nk_rect getRect(
	GUITransformComponent *guiTransform,
	real32 width,
	real32 height)
{
	struct nk_rect rect;

	rect.x = ((guiTransform->position.x + 1.0f) / 2.0f) * width;
	rect.y = ((1.0f - guiTransform->position.y) / 2.0f) * height;

	rect.w = guiTransform->size.x * width;
	rect.h = guiTransform->size.y * height;

	switch (guiTransform->anchor)
	{
		case ANCHOR_TOP:
		case ANCHOR_CENTER:
		case ANCHOR_BOTTOM:
			rect.x -= rect.w / 2.0f;
			break;
		case ANCHOR_TOP_RIGHT:
		case ANCHOR_RIGHT:
		case ANCHOR_BOTTOM_RIGHT:
			rect.x -= rect.w;
			break;
		default:
			break;
	}

	switch (guiTransform->anchor)
	{
		case ANCHOR_LEFT:
		case ANCHOR_CENTER:
		case ANCHOR_RIGHT:
			rect.y -= rect.h / 2.0f;
			break;
		case ANCHOR_BOTTOM_LEFT:
		case ANCHOR_BOTTOM:
		case ANCHOR_BOTTOM_RIGHT:
			rect.y -= rect.h;
			break;
		default:
			break;
	}

	return rect;
}

void addWidgets(
	Scene *scene,
	UUID entity,
	real32 panelWidth,
	real32 panelHeight)
{
	// TODO: Add all types of widgets
	do
	{
		GUITransformComponent *guiTransform = sceneGetComponentFromEntity(
			scene,
			entity,
			guiTransformComponentID);
		WidgetListComponent *widgetList = sceneGetComponentFromEntity(
			scene,
			entity,
			widgetListComponentID);

		if (widgetList && guiTransform)
		{
			nk_layout_space_push(
				&ctx,
				getRect(guiTransform, panelWidth, panelHeight));

			addText(scene, entity);
			addButton(scene, entity);

			entity = widgetList->nextWidget;
		}
		else
		{
			break;
		}
	} while (true);
}

void addText(Scene *scene, UUID entity)
{
	TextComponent *text = sceneGetComponentFromEntity(
		scene,
		entity,
		textComponentID);

	if (!text)
	{
		return;
	}

	if (text->alignment == TEXT_ALIGNMENT_WRAP)
	{
		nk_label_colored_wrap(
			&ctx,
			text->text,
			getColor(&text->color));
	}
	else
	{
		enum nk_text_align alignment = (enum nk_text_align)NK_TEXT_CENTERED;
		switch (text->alignment)
		{
			case TEXT_ALIGNMENT_TOP_LEFT:
				alignment = NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_LEFT;
				break;
			case TEXT_ALIGNMENT_TOP:
				alignment = NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_CENTERED;
				break;
			case TEXT_ALIGNMENT_TOP_RIGHT:
				alignment = NK_TEXT_ALIGN_TOP | NK_TEXT_ALIGN_RIGHT;
				break;
			case TEXT_ALIGNMENT_LEFT:
				alignment = (enum nk_text_align)NK_TEXT_LEFT;
				break;
			case TEXT_ALIGNMENT_RIGHT:
				alignment = (enum nk_text_align)NK_TEXT_RIGHT;
				break;
			case TEXT_ALIGNMENT_BOTTOM_LEFT:
				alignment = NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_LEFT;
				break;
			case TEXT_ALIGNMENT_BOTTOM:
				alignment = NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_CENTERED;
				break;
			case TEXT_ALIGNMENT_BOTTOM_RIGHT:
				alignment = NK_TEXT_ALIGN_BOTTOM | NK_TEXT_ALIGN_RIGHT;
				break;
			default:
				break;
		}

		nk_label_colored(
			&ctx,
			text->text,
			alignment,
			getColor(&text->color));
	}
}

void addButton(Scene *scene, UUID entity)
{
	ButtonComponent *button = sceneGetComponentFromEntity(
		scene,
		entity,
		buttonComponentID);

	if (!button)
	{
		return;
	}

	button->pressedLastFrame = button->pressed;
	button->pressed = nk_button_label(&ctx, button->text);
}