#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/font.h"
#include "asset_management/image.h"

#include "components/component_types.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/component.h"
#include "ECS/scene.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

internal UUID defaultFontEntityID = {};

internal UUID guiTransformComponentID = {};
internal UUID panelComponentID = {};
internal UUID widgetComponentID = {};
internal UUID fontComponentID = {};
internal UUID textComponentID = {};
internal UUID imageComponentID = {};
internal UUID buttonComponentID = {};
internal UUID textFieldComponentID = {};
internal UUID progressBarComponentID = {};
internal UUID sliderComponentID = {};

extern uint32 guiRefCount;

extern struct nk_context ctx;
struct nk_buffer cmds;

#define DEFAULT_FONT "default_font"
#define DEFAULT_FONT_SIZE 18

internal FontComponent *defaultFontComponent;
internal Font defaultFont;
internal bool updateDefaultFont;

#define WIDGET_BACKGROUND "widget_background"

internal Image widgetBackground;

internal int32 previousViewportHeight = 0;

extern int32 viewportWidth;
extern int32 viewportHeight;

internal struct nk_convert_config nkConfig;

#define MAX_GUI_VERTEX_COUNT 512 * 2048
#define MAX_GUI_INDEX_COUNT 128 * 2048

#define NUM_GUI_VERTEX_ATTRIBUTES 3

typedef struct gui_vertex_t
{
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

GLuint guiVertexBuffer;
GLuint guiVertexArray;
GLuint guiIndexBuffer;

typedef struct panel_layer_t
{
	UUID entity;
	PanelComponent *panel;
} PanelLayer;

internal void quickSortPanelLayers(
	PanelLayer *panelLayers,
	int32 left,
	int32 right);

internal Font getEntityFont(
	Scene *scene,
	UUID entity,
	FontComponent *fallbackFontComponent,
	Font *fallBackFont,
	FontComponent **fontComponent);
internal struct nk_color getColor(kmVec4 *color);
internal struct nk_rect getRect(
	GUITransformComponent *guiTransform,
	real32 width,
	real32 height);

internal void addWidgets(
	Scene *scene,
	UUID entity,
	UUID panel,
	real32 panelWidth,
	real32 panelHeight);
internal void addText(TextComponent *text, FontComponent *font);
internal void addImage(
	ImageComponent *imageComponent,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight);
internal void addButton(
	ButtonComponent *button,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight);
internal void addTextField(TextFieldComponent *textField);
internal void addProgressBar(
	ProgressBarComponent *progressBar,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight);
internal void addSlider(
	SliderComponent *slider,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight);

internal void fillCommandBuffer(void);

internal void initGUISystem(Scene *scene)
{
	if (guiRefCount == 0)
	{
		LOG("Initializing GUI...\n");

		if (!nk_init_default(&ctx, NULL))
		{
			LOG("Failed to initialize the GUI\n");
		}
		else
		{
			nk_buffer_init_default(&cmds);

			loadImage(WIDGET_BACKGROUND, false);
			widgetBackground = getImage(WIDGET_BACKGROUND);

			memset(&nkConfig, 0, sizeof(struct nk_convert_config));
			nkConfig.shape_AA = NK_ANTI_ALIASING_ON;
			nkConfig.line_AA = NK_ANTI_ALIASING_ON;
			nkConfig.vertex_layout = vertex_layout;
			nkConfig.vertex_size = sizeof(GUIVertex);
			nkConfig.vertex_alignment = NK_ALIGNOF(GUIVertex);
			nkConfig.circle_segment_count = 22;
			nkConfig.curve_segment_count = 22;
			nkConfig.arc_segment_count = 22;
			nkConfig.global_alpha = 1.0f;

			// TODO: Move styles to components

			ctx.style.window.padding.x = 0.0f;
			ctx.style.window.padding.y = 0.0f;

			ctx.style.button.border = 0.0f;
			ctx.style.button.rounding = 0.0f;

			nk_button_set_behavior(&ctx, NK_BUTTON_REPEATER);

			glGenBuffers(1, &guiVertexBuffer);
			glGenVertexArrays(1, &guiVertexArray);

			uint32 bufferIndex = 0;

			glBindBuffer(GL_ARRAY_BUFFER, guiVertexBuffer);
			glBindVertexArray(guiVertexArray);

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

			glGenBuffers(1, &guiIndexBuffer);

			LOG("Successfully initialized GUI\n");
		}
	}

	updateDefaultFont = true;
	guiRefCount++;
}

internal void beginGUISystem(Scene *scene, real64 dt)
{
	if (viewportHeight != previousViewportHeight)
	{
		ComponentDataTable *fontComponents =
			*(ComponentDataTable**)hashMapGetData(
				scene->componentTypes,
				&fontComponentID);

		for (ComponentDataTableIterator itr = cdtGetIterator(fontComponents);
			 !cdtIteratorAtEnd(itr);
			 cdtMoveIterator(&itr))
		{
			FontComponent *fontComponent = cdtIteratorGetData(itr);
			if (strlen(getFont(
					fontComponent->name,
					fontComponent->size,
					fontComponent->autoScaling).name.string) == 0)
			{
				loadFont(
					fontComponent->name,
					fontComponent->size,
					fontComponent->autoScaling);
			}
		}

		updateDefaultFont = true;
	}

	defaultFontComponent = sceneGetComponentFromEntity(
		scene,
		defaultFontEntityID,
		fontComponentID);

	defaultFont = getFont(
		defaultFontComponent->name,
		defaultFontComponent->size,
		defaultFontComponent->autoScaling);

	if (updateDefaultFont)
	{
		if (strlen(defaultFont.name.string) > 0)
		{
			updateDefaultFont = false;
			nk_style_set_font(&ctx, &defaultFont.font->handle);
			nkConfig.null = defaultFont.null;
		}
		else
		{
			return;
		}
	}

	previousViewportHeight = viewportHeight;

	widgetBackground = getImage(WIDGET_BACKGROUND);

	glBindVertexArray(guiVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, guiVertexBuffer);

	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(GUIVertex) * MAX_GUI_VERTEX_COUNT,
		NULL,
		GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, guiIndexBuffer);

	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16) * MAX_GUI_INDEX_COUNT,
		NULL,
		GL_STREAM_DRAW);

	uint32 numPanelLayers = 0, maxPanelLayers = 0;
	PanelLayer *panelLayers = NULL;

	ComponentDataTable *panelComponents =
		*(ComponentDataTable**)hashMapGetData(
			scene->componentTypes,
			&panelComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(panelComponents);
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		PanelComponent *panelComponent = cdtIteratorGetData(itr);
		if (!panelComponent->enabled)
		{
			continue;
		}

		if (numPanelLayers == maxPanelLayers)
		{
			maxPanelLayers += 512;
			panelLayers = realloc(
				panelLayers,
				maxPanelLayers * sizeof(PanelLayer));
		}

		PanelLayer *panelLayer = &panelLayers[numPanelLayers++];
		panelLayer->panel = panelComponent;
		panelLayer->entity = cdtIteratorGetUUID(itr);
	}

	quickSortPanelLayers(panelLayers, 0, numPanelLayers - 1);

	for (uint32 i = 0; i < numPanelLayers; i++)
	{
		PanelLayer *panelLayer = &panelLayers[i];

		PanelComponent *panel = panelLayer->panel;
		UUID entityID = panelLayer->entity;

		ctx.style.window.fixed_background = nk_style_item_color(
		getColor(&panel->color));

		GUITransformComponent *guiTransform = sceneGetComponentFromEntity(
			scene,
			entityID,
			guiTransformComponentID);

		struct nk_rect rect = getRect(
			guiTransform,
			viewportWidth,
			viewportHeight);

		char windowTitle[1024];
		sprintf(windowTitle, "%s_%s", scene->name, entityID.string);

		if (nk_begin(
			&ctx,
			windowTitle,
			rect,
			NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_space_begin(&ctx, NK_STATIC, 0, INT_MAX);
			addWidgets(scene, panel->firstWidget, entityID, rect.w, rect.h);
			nk_layout_space_end(&ctx);
		}

		nk_end(&ctx);
	}

	free(panelLayers);
}

internal void endGUISystem(Scene *scene, real64 dt)
{
	if (!updateDefaultFont)
	{
		fillCommandBuffer();

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

internal void shutdownGUISystem(Scene *scene)
{
	if (--guiRefCount == 0)
	{
		LOG("Shutting down GUI...\n");

		nk_free(&ctx);
		nk_buffer_free(&cmds);

		glBindVertexArray(guiVertexArray);
		glDeleteBuffers(1, &guiVertexBuffer);
		glDeleteBuffers(1, &guiIndexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &guiVertexArray);

		LOG("Successfully shut down GUI\n");
	}
}

System createGUISystem(void)
{
	defaultFontEntityID = idFromName("default_font");

	guiTransformComponentID = idFromName("gui_transform");
	panelComponentID = idFromName("panel");
	widgetComponentID = idFromName("widget");
	fontComponentID = idFromName("font");
	textComponentID = idFromName("text");
	imageComponentID = idFromName("image");
	buttonComponentID = idFromName("button");
	textFieldComponentID = idFromName("text_field");
	progressBarComponentID = idFromName("progress_bar");
	sliderComponentID = idFromName("slider");

	System system = {};

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &guiTransformComponentID);
	listPushFront(&system.componentTypes, &panelComponentID);

	system.init = &initGUISystem;
	system.begin = &beginGUISystem;
	system.end = &endGUISystem;
	system.shutdown = &shutdownGUISystem;

	return system;
}

void quickSortPanelLayers(PanelLayer *panelLayers, int32 left, int32 right)
{
	if (left >= right)
	{
		return;
	}

	PanelLayer pivot = panelLayers[right];
	int32 count = left;

	for (int32 i = left; i <= right; i++)
	{
		if (panelLayers[i].panel->layer >= pivot.panel->layer)
		{
			PanelLayer panelLayer = panelLayers[count];
			panelLayers[count] = panelLayers[i];
			panelLayers[i] = panelLayer;

			count++;
		}
	}

	quickSortPanelLayers(panelLayers, left, count - 2);
	quickSortPanelLayers(panelLayers, count, right);
}

Font getEntityFont(
	Scene *scene,
	UUID entity,
	FontComponent *fallbackFontComponent,
	Font *fallBackFont,
	FontComponent **fontComponent)
{
	FontComponent *entityFontComponent = sceneGetComponentFromEntity(
		scene,
		entity,
		fontComponentID);

	if (fontComponent)
	{
		if (entityFontComponent)
		{
			*fontComponent = entityFontComponent;
		}
		else
		{
			*fontComponent = fallbackFontComponent;
		}
	}

	if (entityFontComponent)
	{
		Font font = getFont(
			entityFontComponent->name,
			entityFontComponent->size,
			entityFontComponent->autoScaling);

		if (strlen(font.name.string) > 0)
		{
			return font;
		}
	}

	return *fallBackFont;
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

	kmVec2 position;
	if (guiTransform->positionMode == POSITION_LAYOUT_MODE_SCREEN)
	{
		kmVec2Fill(
			&position,
			guiTransform->position.x,
			guiTransform->position.y);
	}
	else
	{
		kmVec2Fill(
			&position,
			((guiTransform->position.x + 1.0f) / 2.0f) * width,
			((1.0f - guiTransform->position.y) / 2.0f) * height);
	}

	rect.x = position.x;
	rect.y = position.y;

	kmVec2 sizeMultiplier;
	if (guiTransform->sizeMode == SIZE_LAYOUT_MODE_PIXELS)
	{
		kmVec2Fill(&sizeMultiplier, 1.0f, 1.0f);
	}
	else
	{
		kmVec2Fill(&sizeMultiplier, width, height);
	}

	rect.w = guiTransform->size.x * sizeMultiplier.x;
	rect.h = guiTransform->size.y * sizeMultiplier.y;;

	switch (guiTransform->pivot)
	{
		case PIVOT_TOP:
		case PIVOT_CENTER:
		case PIVOT_BOTTOM:
			rect.x -= rect.w / 2.0f;
			break;
		case PIVOT_TOP_RIGHT:
		case PIVOT_RIGHT:
		case PIVOT_BOTTOM_RIGHT:
			rect.x -= rect.w;
			break;
		default:
			break;
	}

	switch (guiTransform->pivot)
	{
		case PIVOT_LEFT:
		case PIVOT_CENTER:
		case PIVOT_RIGHT:
			rect.y -= rect.h / 2.0f;
			break;
		case PIVOT_BOTTOM_LEFT:
		case PIVOT_BOTTOM:
		case PIVOT_BOTTOM_RIGHT:
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
	UUID panel,
	real32 panelWidth,
	real32 panelHeight)
{
	FontComponent *panelFontComponent;
	Font panelFont = getEntityFont(
		scene,
		panel,
		defaultFontComponent,
		&defaultFont,
		&panelFontComponent);

	do
	{
		GUITransformComponent *guiTransform = sceneGetComponentFromEntity(
			scene,
			entity,
			guiTransformComponentID);
		WidgetComponent *widget = sceneGetComponentFromEntity(
			scene,
			entity,
			widgetComponentID);

		if (widget && guiTransform)
		{
			if (widget->enabled)
			{
				FontComponent *fontComponent;
				Font font = getEntityFont(
					scene,
					entity,
					panelFontComponent,
					&panelFont,
					&fontComponent);

				nk_style_set_font(&ctx, &font.font->handle);

				struct nk_rect rect = getRect(
					guiTransform,
					panelWidth,
					panelHeight);

				nk_layout_space_push(&ctx, rect);
				nk_image_color(
					&ctx,
					nk_image_id(widgetBackground.id),
					getColor(&widget->backgroundColor));

				TextComponent *text = sceneGetComponentFromEntity(
					scene,
					entity,
					textComponentID);
				ImageComponent *image = sceneGetComponentFromEntity(
					scene,
					entity,
					imageComponentID);
				ButtonComponent *button = sceneGetComponentFromEntity(
					scene,
					entity,
					buttonComponentID);
				TextFieldComponent *textField = sceneGetComponentFromEntity(
					scene,
					entity,
					textFieldComponentID);
				ProgressBarComponent *progressBar = sceneGetComponentFromEntity(
					scene,
					entity,
					progressBarComponentID);
				SliderComponent *slider = sceneGetComponentFromEntity(
					scene,
					entity,
					sliderComponentID);

				if (!image && !progressBar && !slider)
				{
					nk_layout_space_push(&ctx, rect);
				}

				if (text)
				{
					addText(text, fontComponent);
				}
				else if (image)
				{
					addImage(image, guiTransform, panelWidth, panelHeight);
				}
				else if (button)
				{
					addButton(button, guiTransform, panelWidth, panelHeight);
				}
				else if (textField)
				{
					addTextField(textField);
				}
				else if (progressBar)
				{
					addProgressBar(
						progressBar,
						guiTransform,
						panelWidth,
						panelHeight);
				}
				else if (slider)
				{
					addSlider(slider, guiTransform, panelWidth, panelHeight);
				}
			}

			entity = widget->nextWidget;
		}
		else
		{
			break;
		}
	} while (true);
}

void addText(TextComponent *text, FontComponent *font)
{
	if (text->alignment == TEXT_ALIGNMENT_WRAP)
	{
		nk_label_colored_wrap(
			&ctx,
			text->text,
			getColor(&font->color));
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
			getColor(&font->color));
	}
}

void addImage(
	ImageComponent *imageComponent,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight)
{
	Image image = getImage(imageComponent->name);

	if (strlen(image.name.string) == 0)
	{
		return;
	}

	GUITransformComponent imageTransform;
	imageTransform.position = imageComponent->position;
	imageTransform.positionMode = imageComponent->positionMode;
	imageTransform.pivot = imageComponent->pivot;

	struct nk_rect widgetRect = getRect(guiTransform, panelWidth, panelHeight);

	real32 width = (image.data.width / widgetRect.w) * imageComponent->scale.x;
	real32 height =
		(image.data.height / widgetRect.h) * imageComponent->scale.x;

	if (width > 1.0f)
	{
		height /= width;
		width = 1.0f;
	}

	if (height > 1.0f)
	{
		width /= height;
		height = 1.0f;
	}

	kmVec2Fill(&imageTransform.size, width, height);
	imageTransform.sizeMode = SIZE_LAYOUT_MODE_RATIO;

	struct nk_rect rect = getRect(&imageTransform, widgetRect.w, widgetRect.h);
	rect.x += widgetRect.x;
	rect.y += widgetRect.y;

	nk_layout_space_push(&ctx, rect);

	nk_image_color(
		&ctx,
		nk_image_id(image.id),
		getColor(&imageComponent->color));
}

void addButton(
	ButtonComponent *button,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight)
{
	bool held = button->held;
	button->held = nk_button_label(&ctx, button->text);
	button->pressed = !held && button->held;
	button->released = held && !button->held;

	GUITransformComponent buttonTransform = *guiTransform;
	buttonTransform.pivot = PIVOT_TOP_LEFT;
	struct nk_rect rect = getRect(&buttonTransform, panelWidth, panelHeight);
	button->hovered = nk_input_is_mouse_hovering_rect(&ctx.input, rect);
}

void addTextField(TextFieldComponent *textField)
{
	nk_edit_string_zero_terminated(
		&ctx,
		NK_EDIT_ALLOW_TAB |
		NK_EDIT_SELECTABLE |
		NK_EDIT_ALWAYS_INSERT_MODE |
		NK_EDIT_GOTO_END_ON_ACTIVATE,
		textField->text,
		4096,
		NULL);
}

void addProgressBar(
	ProgressBarComponent *progressBar,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight)
{
	struct nk_rect widgetRect = getRect(guiTransform, panelWidth, panelHeight);

	nk_layout_space_push(&ctx, widgetRect);
	nk_image_color(
		&ctx,
		nk_image_id(widgetBackground.id),
		getColor(&progressBar->backgroundColor));

	struct nk_rect rect = widgetRect;

	if (progressBar->reversed)
	{
		rect.x += (1.0f - progressBar->value) * rect.w;
	}

	rect.w *= progressBar->value;

	nk_layout_space_push(&ctx, rect);
	nk_image_color(
		&ctx,
		nk_image_id(widgetBackground.id),
		getColor(&progressBar->color));
}

void addSlider(
	SliderComponent *slider,
	GUITransformComponent *guiTransform,
	real32 panelWidth,
	real32 panelHeight)
{
	GUITransformComponent sliderTransform = *guiTransform;
	sliderTransform.size.y *= 6.0f * slider->height;

	struct nk_rect rect = getRect(&sliderTransform, panelWidth, panelHeight);
	nk_layout_space_push(&ctx, rect);

	real32 widgetHeight = guiTransform->sizeMode == SIZE_LAYOUT_MODE_PIXELS ?
		guiTransform->size.y : panelHeight * guiTransform->size.y;
	real32 sliderHeight = widgetHeight * slider->height;

	ctx.style.slider.cursor_size.x = slider->buttonSize * sliderHeight;
	ctx.style.slider.cursor_size.y = slider->buttonSize * sliderHeight;

	real32 widgetWidth = guiTransform->sizeMode == SIZE_LAYOUT_MODE_PIXELS ?
		guiTransform->size.x : panelWidth * guiTransform->size.x;
	real32 padding = (widgetWidth - (widgetWidth * slider->length)) / 2;
	ctx.style.slider.padding = nk_vec2(padding, 0.0f);

	ctx.style.slider.cursor_normal = nk_style_item_color(
		getColor(&slider->buttonColor));
	ctx.style.slider.cursor_active = ctx.style.slider.cursor_normal;
	ctx.style.slider.cursor_hover = ctx.style.slider.cursor_normal;

	ctx.style.slider.bar_filled = getColor(&slider->fillColor);

	ctx.style.slider.bar_normal = getColor(&slider->backgroundColor);
	ctx.style.slider.bar_active = ctx.style.slider.bar_normal;
	ctx.style.slider.bar_hover = ctx.style.slider.bar_normal;

	nk_slider_int(
		&ctx,
		slider->minValue,
		&slider->value,
		slider->maxValue,
		slider->stepSize);
}

void fillCommandBuffer(void)
{
	void *vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	void *indices = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	struct nk_buffer guiVertexBufferData;
	nk_buffer_init_fixed(
		&guiVertexBufferData,
		vertices,
		sizeof(GUIVertex) * MAX_GUI_VERTEX_COUNT);

	struct nk_buffer guiIndexBufferData;
	nk_buffer_init_fixed(
		&guiIndexBufferData,
		indices,
		sizeof(uint16) * MAX_GUI_INDEX_COUNT);

	if (nk_convert(
		&ctx,
		&cmds,
		&guiVertexBufferData,
		&guiIndexBufferData,
		&nkConfig) != NK_CONVERT_SUCCESS)
	{
		LOG("Failed to fill GUI command buffer\n");
	}

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	nk_buffer_free(&guiVertexBufferData);
	nk_buffer_free(&guiIndexBufferData);
}