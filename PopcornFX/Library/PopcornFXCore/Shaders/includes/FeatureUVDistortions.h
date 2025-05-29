#pragma once

#include "PKSurface.h"
#include "FeatureAlphaMasks.h"
#include "FeatureTransformUVs.h"

#if defined(HAS_UVDistortions)
void		ApplyUVDistortion( INOUT(SFragGeometry)fGeom, IN(float)animationCursor1, IN(float)animationCursor2, INOUT(SMaskDistortion) fMaskDist FS_ARGS)
{
    // move to InitializeMaskDistortion
    float   angle1 = GET_CONSTANT(Material, UVDistortions_Distortion1RotationSpeed) * animationCursor1;
    float	sinR1 = sin(angle1);
	float	cosR1 = cos(angle1);
	mat2	UVRotation1 = mat2(cosR1, sinR1, -sinR1, cosR1);
    vec2    translation1 = GET_CONSTANT(Material, UVDistortions_Distortion1TranslationSpeed) * animationCursor1;
    vec2    scale1 = GET_CONSTANT(Material, UVDistortions_Distortion1Scale);

    vec2 uv1 = TransformUV(fGeom.m_RawUV0 - 0.5, scale1, UVRotation1, translation1 + scale1 * 0.5);
    fMaskDist.m_UVDistortion1 = SAMPLE(UVDistortions_Distortion1Map, uv1).rg * GET_CONSTANT(Material, UVDistortions_Distortion1Intensity) - 0.5f;

    float   angle2 = GET_CONSTANT(Material, UVDistortions_Distortion2RotationSpeed) * animationCursor2;
    float	sinR2 = sin(angle2);
	float	cosR2 = cos(angle2);
	mat2	UVRotation2 = mat2(cosR2, sinR2, -sinR2, cosR2);
    vec2    translation2 = GET_CONSTANT(Material, UVDistortions_Distortion2TranslationSpeed) * animationCursor2;
    vec2    scale2 = GET_CONSTANT(Material, UVDistortions_Distortion2Scale);

    vec2 uv2 = TransformUV(fGeom.m_RawUV0 - 0.5, scale2, UVRotation2, translation2 + scale2 * 0.5);
	fMaskDist.m_UVDistortion2 = SAMPLE(UVDistortions_Distortion2Map, uv2).rg * GET_CONSTANT(Material, UVDistortions_Distortion2Intensity) - 0.5f;
	
}
#endif
