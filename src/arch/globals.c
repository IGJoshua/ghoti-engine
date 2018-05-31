#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

Model *models;
uint32 numModels = 0;
uint32 modelsCapacity = 0;

Texture *textures;
uint32 numTextures = 0;
uint32 texturesCapacity = 0;

Shader vertShader;
Shader fragShader;

ShaderPipeline pipeline;

Uniform modelUniform;
Uniform viewUniform;
Uniform projectionUniform;

Uniform diffuseTextureUniform;
