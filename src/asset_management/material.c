#include "asset_management/material.h"
#include "asset_management/texture.h"

#include <assimp/scene.h>
#include <malloc.h>

int32 loadMaterial(
	const struct aiMaterial *materialData,
	Material *material)
{
	material->type = MATERIAL_TYPE_DEBUG;

	struct aiString textureName;

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0),
		&textureName) == AI_SUCCESS)
	{
		material->diffuseTexture = malloc(textureName.length + 1);
		strcpy(material->diffuseTexture, textureName.data);
		loadTexture(&textureName, TEXTURE_TYPE_DIFFUSE);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0),
		&textureName) == AI_SUCCESS)
	{
		material->specularTexture = malloc(textureName.length + 1);
		strcpy(material->specularTexture, textureName.data);
		loadTexture(&textureName, TEXTURE_TYPE_SPECULAR);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0),
		&textureName) == AI_SUCCESS)
	{
		material->normalMap = malloc(textureName.length + 1);
		strcpy(material->normalMap, textureName.data);
		loadTexture(&textureName, TEXTURE_TYPE_NORMAL);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0),
		&textureName) == AI_SUCCESS)
	{
		material->emissiveMap = malloc(textureName.length + 1);
		strcpy(material->emissiveMap, textureName.data);
		loadTexture(&textureName, TEXTURE_TYPE_EMISSIVE);
	}

	struct aiColor4D materialValue;

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_DIFFUSE,
		&materialValue) == AI_SUCCESS)
	{
		material->diffuseValue.x = materialValue.r;
		material->diffuseValue.y = materialValue.g;
		material->diffuseValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_SPECULAR,
		&materialValue) == AI_SUCCESS)
	{
		material->specularValue.x = materialValue.r;
		material->specularValue.y = materialValue.g;
		material->specularValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_AMBIENT,
		&materialValue) == AI_SUCCESS)
	{
		material->ambientValue.x = materialValue.r;
		material->ambientValue.y = materialValue.g;
		material->ambientValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_EMISSIVE,
		&materialValue) == AI_SUCCESS)
	{
		material->emissiveValue.x = materialValue.r;
		material->emissiveValue.y = materialValue.g;
		material->emissiveValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_TRANSPARENT,
		&materialValue) == AI_SUCCESS)
	{
		material->transparentValue.x = materialValue.r;
		material->transparentValue.y = materialValue.g;
		material->transparentValue.z = materialValue.b;
	}

	real32 materialConstant;

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_SHININESS,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->specularPower = materialConstant;
	}

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_SHININESS_STRENGTH,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->specularScale = materialConstant;
	}

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_OPACITY,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->opacity = materialConstant;
	}

	return 0;
}

int32 freeMaterial(Material *material)
{
	if (freeTexture(material->diffuseTexture) == -1)
	{
		return -1;
	}

	if (freeTexture(material->specularTexture) == -1)
	{
		return -1;
	}

	if (freeTexture(material->normalMap) == -1)
	{
		return -1;
	}

	if (freeTexture(material->emissiveMap) == -1)
	{
		return -1;
	}

	free(material->diffuseTexture);
	free(material->specularTexture);
	free(material->normalMap);
	free(material->emissiveMap);

	return 0;
}
