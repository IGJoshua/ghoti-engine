#include "asset_management/animation.h"

#include "file/utilities.h"

#include <malloc.h>

internal void freeBone(Bone *bone);
internal void freeSkeleton(Skeleton *skeleton);

int32 loadAnimations(
	uint32 *numAnimations,
	Animation **animations,
	Skeleton *skeleton,
	FILE *file)
{
	bool hasAnimations;
	fread(&hasAnimations, sizeof(bool), 1, file);

	if (hasAnimations)
	{
		fread(&skeleton->numBoneOffsets, sizeof(uint32), 1, file);
		skeleton->boneOffsets = calloc(
			skeleton->numBoneOffsets,
			sizeof(BoneOffset));

		for (uint32 i = 0; i < skeleton->numBoneOffsets; i++)
		{
			BoneOffset *boneOffset = &skeleton->boneOffsets[i];
			boneOffset->name = readUUID(file);
			boneOffset->transform = readTransform(file);
		}

		fread(numAnimations, sizeof(uint32), 1, file);
		*animations = calloc(*numAnimations, sizeof(Animation));

		for (uint32 i = 0; i < *numAnimations; i++)
		{
			Animation *animation = &(*animations)[i];

			animation->name = readUUID(file);
			fread(&animation->duration, sizeof(real64), 1, file);
			fread(&animation->fps, sizeof(real64), 1, file);

			fread(&animation->numBones, sizeof(uint32), 1, file);
			animation->bones = calloc(animation->numBones, sizeof(Bone));

			for (uint32 j = 0; j < animation->numBones; j++)
			{
				Bone *bone = &animation->bones[j];

				bone->name = readUUID(file);

				fread(&bone->numPositionKeyFrames, sizeof(uint32), 1, file);
				bone->positionKeyFrames = calloc(
					bone->numPositionKeyFrames,
					sizeof(Vec3KeyFrame));

				for (uint32 k = 0; k < bone->numPositionKeyFrames; k++)
				{
					Vec3KeyFrame *keyFrame = &bone->positionKeyFrames[k];
					fread(&keyFrame->time, sizeof(real64), 1, file);
					fread(&keyFrame->value, sizeof(kmVec3), 1, file);
				}

				fread(&bone->numRotationKeyFrames, sizeof(uint32), 1, file);
				bone->rotationKeyFrames = calloc(
					bone->numRotationKeyFrames,
					sizeof(QuaternionKeyFrame));

				for (uint32 k = 0; k < bone->numRotationKeyFrames; k++)
				{
					QuaternionKeyFrame *keyFrame = &bone->rotationKeyFrames[k];
					fread(&keyFrame->time, sizeof(real64), 1, file);
					fread(&keyFrame->value, sizeof(kmQuaternion), 1, file);
				}

				fread(&bone->numScaleKeyFrames, sizeof(uint32), 1, file);
				bone->scaleKeyFrames = calloc(
					bone->numScaleKeyFrames,
					sizeof(Vec3KeyFrame));

				for (uint32 k = 0; k < bone->numScaleKeyFrames; k++)
				{
					Vec3KeyFrame *keyFrame = &bone->scaleKeyFrames[k];
					fread(&keyFrame->time, sizeof(real64), 1, file);
					fread(&keyFrame->value, sizeof(kmVec3), 1, file);
				}
			}
		}
	}
	else
	{
		*numAnimations = 0;
	}

	return 0;
}

void freeAnimations(
	uint32 numAnimations,
	Animation *animations,
	Skeleton *skeleton)
{
	freeSkeleton(skeleton);

	for (uint32 i = 0; i < numAnimations; i++)
	{
		Animation *animation = &animations[i];
		for (uint32 j = 0; j < animation->numBones; j++)
		{
			freeBone(&animation->bones[j]);
		}

		free(animation->bones);
	}

	free(animations);
}

void freeBone(Bone *bone)
{
	free(bone->positionKeyFrames);
	free(bone->rotationKeyFrames);
	free(bone->scaleKeyFrames);
}

void freeSkeleton(Skeleton *skeleton)
{
	free(skeleton->boneOffsets);
}