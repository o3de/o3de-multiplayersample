#pragma once

#include "PKSurface.h"
#include "FeatureTransformUVs.h"

#if defined(HAS_TransformedNoises) || defined(HAS_UVDistortions) 

void        InitializeNoiseDist(INOUT(SNoiseDistortion) fNoiseDistortion FS_ARGS)
{
#   if defined(HAS_UVDistortions)
    fNoiseDistortion.m_UVDistortion1 = vec2(0.f, 0.f);
    fNoiseDistortion.m_UVDistortion2 = vec2(0.f, 0.f);
#   endif

#   if defined(HAS_TransformedNoises)
    fNoiseDistortion.m_NoiseSample1 = 0.f;
    fNoiseDistortion.m_NoiseSample2 = 0.f;
#   endif
}

#   if defined(HAS_TransformedNoises)
void        ApplyTransformedNoises(IN(SFragGeometry) fGeom, IN(float) animationCursor1, IN(float) animationCursor2,  INOUT(SNoiseDistortion) fNoiseDistortion FS_ARGS)
{
#   if defined(HAS_UVDistortions)
    vec2 uvNoise1 = fNoiseDistortion.m_UVDistortion1 * 0.1f + fNoiseDistortion.m_UVDistortion2 * 0.01f + fGeom.m_RawUV0;
    vec2 uvNoise2 = fNoiseDistortion.m_UVDistortion1 * 0.01f + fNoiseDistortion.m_UVDistortion2 * 0.1f + fGeom.m_RawUV0;
#   else
    vec2 uvNoise1 = fGeom.m_RawUV0;
    vec2 uvNoise2 = fGeom.m_RawUV0;
#   endif

    float   angle1 = GET_CONSTANT(Material, TransformedNoises_Noise1RotationSpeed) * animationCursor1;
    float	sinR1 = sin(angle1);
	float	cosR1 = cos(angle1);
	mat2	UVRotation1 = mat2(cosR1, sinR1, -sinR1, cosR1);

    vec2 translation1 = GET_CONSTANT(Material, TransformedNoises_Noise1TranslationSpeed) * animationCursor1;
    vec2 scale1 = GET_CONSTANT(Material, TransformedNoises_Noise1Scale);
    uvNoise1 = TransformUV(uvNoise1 - 0.5, scale1, UVRotation1, translation1 + scale1 * 0.5);
    fNoiseDistortion.m_NoiseSample1 = SAMPLE(TransformedNoises_Noise1Map, uvNoise1).r * GET_CONSTANT(Material, TransformedNoises_Noise1Intensity);
    fNoiseDistortion.m_NoiseSample1 = mix(fNoiseDistortion.m_NoiseSample1, 1.f, 1 - GET_CONSTANT(Material, TransformedNoises_Noise1Weight)); // Do this on CPU

    float   angle2 = GET_CONSTANT(Material, TransformedNoises_Noise2RotationSpeed) * animationCursor2;
    float	sinR2 = sin(angle2);
	float	cosR2 = cos(angle2);
	mat2	UVRotation2 = mat2(cosR2, sinR2, -sinR2, cosR2);

    vec2 translation2 = GET_CONSTANT(Material, TransformedNoises_Noise2TranslationSpeed) * animationCursor2;
    vec2 scale2 = GET_CONSTANT(Material, TransformedNoises_Noise2Scale);
    uvNoise2 = TransformUV(uvNoise2 - 0.5, scale2, UVRotation2, translation2 + scale2 * 0.5);
    fNoiseDistortion.m_NoiseSample2 = SAMPLE(TransformedNoises_Noise2Map, uvNoise2).r * GET_CONSTANT(Material, TransformedNoises_Noise2Intensity);
    fNoiseDistortion.m_NoiseSample2 = mix(fNoiseDistortion.m_NoiseSample2, 1.f, 1 - GET_CONSTANT(Material, TransformedNoises_Noise2Weight)); // Do this on CPU
}
#   endif
#endif

#if defined(HAS_TransformedNoises) || defined(HAS_UVDistortions)
void        ApplyFinalAlpha(IN(SFragmentInput)fInput, IN(SFragGeometry)fGeom, INOUT(SFragSurface) fSurf, IN(SNoiseDistortion) fNoiseDistortion FS_ARGS)
#else
void        ApplyFinalAlpha(IN(SFragmentInput)fInput, IN(SFragGeometry)fGeom, INOUT(SFragSurface) fSurf FS_ARGS)
#endif
{
    float alphaProduct = 1;
#   if defined(HAS_Dissolve)
    alphaProduct *= SAMPLE(Dissolve_DissolveMap, fGeom.m_RawUV0).r * fInput.fragDissolve_DissolveAnimationCursor;
#   endif

#   if defined(HAS_TransformedNoises)
    alphaProduct *= fNoiseDistortion.m_NoiseSample1 * fNoiseDistortion.m_NoiseSample2;
#   endif

#   if defined(HAS_Emissive)
    fSurf.m_Emissive *= clamp(alphaProduct, 0.f, 1.f);
#   endif

#   if defined(HAS_Diffuse)
    fSurf.m_Diffuse.a *= alphaProduct;
    fSurf.m_Diffuse.a = clamp(fSurf.m_Diffuse.a, 0.f, 1.f);
#   endif
}
