#pragma once

#include "PKSurface.h"
#include "FeatureTransformUVs.h"

#define SMOOTHSTEP_EPSILON 1.0e-7f

#if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)
#   if defined(HAS_AlphaMasks) || defined(HAS_UVDistortions) 

void        InitializeMaskDistortion(INOUT(SMaskDistortion) fMaskDistortion FS_ARGS)
{
#   if defined(HAS_UVDistortions)
    fMaskDistortion.m_UVDistortion1 = vec2(0.f, 0.f);
    fMaskDistortion.m_UVDistortion2 = vec2(0.f, 0.f);
#   endif

#   if defined(HAS_AlphaMasks)
    fMaskDistortion.m_MaskSample1 = 0.f;
    fMaskDistortion.m_MaskSample2 = 0.f;
#   endif
}

#   if defined(HAS_AlphaMasks)
void        ApplyAlphaMasks(IN(SFragGeometry) fGeom, IN(float) animationCursor1, IN(float) animationCursor2,  INOUT(SMaskDistortion) fMaskDistortion FS_ARGS)
{
#   if defined(HAS_UVDistortions)
    vec2 uvMask1 = fMaskDistortion.m_UVDistortion1 * 0.1f + fMaskDistortion.m_UVDistortion2 * 0.01f + fGeom.m_RawUV0;
    vec2 uvMask2 = fMaskDistortion.m_UVDistortion1 * 0.01f + fMaskDistortion.m_UVDistortion2 * 0.1f + fGeom.m_RawUV0;
#   else
    vec2 uvMask1 = fGeom.m_RawUV0;
    vec2 uvMask2 = fGeom.m_RawUV0;
#   endif

    float   angle1 = GET_CONSTANT(Material, AlphaMasks_Mask1RotationSpeed) * animationCursor1;
    float	sinR1 = sin(angle1);
	float	cosR1 = cos(angle1);
	mat2	UVRotation1 = mat2(cosR1, sinR1, -sinR1, cosR1);

    vec2 translation1 = GET_CONSTANT(Material, AlphaMasks_Mask1TranslationSpeed) * animationCursor1;
    vec2 scale1 = GET_CONSTANT(Material, AlphaMasks_Mask1Scale);
    uvMask1 = TransformUV(uvMask1 - 0.5, scale1, UVRotation1, translation1 + scale1 * 0.5);
    fMaskDistortion.m_MaskSample1 = SAMPLE(AlphaMasks_Mask1Map, uvMask1).r * GET_CONSTANT(Material, AlphaMasks_Mask1Intensity);
    fMaskDistortion.m_MaskSample1 = mix(fMaskDistortion.m_MaskSample1, 1.f, 1 - GET_CONSTANT(Material, AlphaMasks_Mask1Weight)); // Do this on CPU

    float   angle2 = GET_CONSTANT(Material, AlphaMasks_Mask2RotationSpeed) * animationCursor2;
    float	sinR2 = sin(angle2);
	float	cosR2 = cos(angle2);
	mat2	UVRotation2 = mat2(cosR2, sinR2, -sinR2, cosR2);

    vec2 translation2 = GET_CONSTANT(Material, AlphaMasks_Mask2TranslationSpeed) * animationCursor2;
    vec2 scale2 = GET_CONSTANT(Material, AlphaMasks_Mask2Scale);
    uvMask2 = TransformUV(uvMask2 - 0.5, scale2, UVRotation2, translation2 + scale2 * 0.5);
    fMaskDistortion.m_MaskSample2 = SAMPLE(AlphaMasks_Mask2Map, uvMask2).r * GET_CONSTANT(Material, AlphaMasks_Mask2Intensity);
    fMaskDistortion.m_MaskSample2 = mix(fMaskDistortion.m_MaskSample2, 1.f, 1 - GET_CONSTANT(Material, AlphaMasks_Mask2Weight)); // Do this on CPU
}
#   endif
#endif

#if defined(HAS_AlphaMasks) || defined(HAS_UVDistortions)
void        ApplyFinalAlpha(IN(SFragmentInput)fInput, IN(SFragGeometry)fGeom, INOUT(SFragSurface) fSurf, IN(SMaskDistortion) fMaskDistortion FS_ARGS)
#else
void        ApplyFinalAlpha(IN(SFragmentInput)fInput, IN(SFragGeometry)fGeom, INOUT(SFragSurface) fSurf FS_ARGS)
#endif
{
    float alphaProduct = 1;
#   if defined(HAS_AlphaMasks)
    alphaProduct *= fMaskDistortion.m_MaskSample1 * fMaskDistortion.m_MaskSample2;
#   endif

#   if defined(HAS_Emissive)
    fSurf.m_Emissive *= clamp(alphaProduct, 0.f, 1.f);
#   endif

#   if defined(HAS_Diffuse)
    fSurf.m_Diffuse.a *= alphaProduct;
    fSurf.m_Diffuse.a = clamp(fSurf.m_Diffuse.a, 0.f, 1.f);
#   endif

}

#if defined(HAS_Dissolve)
void         ApplyDissolve(IN(float)dissolveCursor, IN(SFragGeometry)fGeom, INOUT(SFragSurface) fSurf FS_PARAMS)
{
    float dissolve = SAMPLE(Dissolve_DissolveMap, fGeom.m_RawUV0).r;
    dissolve = smoothstep(dissolve, dissolve + GET_CONSTANT(Material, Dissolve_DissolveSoftness) + SMOOTHSTEP_EPSILON, dissolveCursor * (1 + SMOOTHSTEP_EPSILON));
    fSurf.m_Emissive *= dissolve;
    fSurf.m_Diffuse.a *= dissolve;
}

#   endif // #if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)
#endif // #   if defined(HAS_AlphaMasks) || defined(HAS_UVDistortions) 

