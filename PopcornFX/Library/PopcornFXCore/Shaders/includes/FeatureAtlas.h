#pragma once

#include "PKSurface.h"

#if defined(HAS_Atlas)

void	ApplyAtlasTexCoords(INOUT(SFragGeometry) fGeom FS_ARGS)
{
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType == 2) // Motion vectors
	{
		vec2	scale = GET_CONSTANT(Material, Atlas_DistortionStrength);
		vec2	curVectors = ((SAMPLE(Atlas_MotionVectorsMap, fGeom.m_UV0).rg * 2.0f) - 1.0f) * scale;
		vec2	nextVectors = ((SAMPLE(Atlas_MotionVectorsMap, fGeom.m_UV1).rg * 2.0f) - 1.0f) * scale;

		curVectors *= fGeom.m_BlendMix;
		nextVectors *= (1.0f - fGeom.m_BlendMix);

		fGeom.m_UV0 = fGeom.m_UV0 - curVectors;
		fGeom.m_UV1 = fGeom.m_UV1 + nextVectors;
	}
}

#endif

vec4	SampleTextureVec4(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	vec4	color = SAMPLEGRAD(toSample, fGeom.m_UV0, fGeom.m_dUVdx, fGeom.m_dUVdy);
#else
	vec4	color = SAMPLE(toSample, fGeom.m_UV0);
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec4	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1, fGeom.m_dUVdx, fGeom.m_dUVdy);
#else
		vec4	color1 =  SAMPLE(toSample, fGeom.m_UV1);
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec4	SampleDistortedTextureVec4(IN(SFragGeometry) fGeom, IN(vec2) distortionUVs, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	vec4	color = SAMPLEGRAD(toSample, fGeom.m_UV0 + distortionUVs, fGeom.m_dUVdx, fGeom.m_dUVdy);
#else
	vec4	color = SAMPLE(toSample, fGeom.m_UV0 + distortionUVs);
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec4	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1 + distortionUVs, fGeom.m_dUVdx, fGeom.m_dUVdy);
#else
		vec4	color1 =  SAMPLE(toSample, fGeom.m_UV1 + distortionUVs);
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec3	SampleTextureVec3(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	vec3	color = SAMPLEGRAD(toSample, fGeom.m_UV0, fGeom.m_dUVdx, fGeom.m_dUVdy).xyz;
#else
	vec3	color = SAMPLE(toSample, fGeom.m_UV0).xyz;
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec3	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1, fGeom.m_dUVdx, fGeom.m_dUVdy).xyz;
#else
		vec3	color1 =  SAMPLE(toSample, fGeom.m_UV1).xyz;
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec3	SampleDistortedTextureVec3(IN(SFragGeometry) fGeom, IN(vec2) distortionUVs, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	vec3	color = SAMPLEGRAD(toSample, fGeom.m_UV0 + distortionUVs, fGeom.m_dUVdx, fGeom.m_dUVdy).xyz;
#else
	vec3	color = SAMPLE(toSample, fGeom.m_UV0 + distortionUVs).xyz;
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec3	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1 + distortionUVs, fGeom.m_dUVdx, fGeom.m_dUVdy).xyz;
#else
		vec3	color1 =  SAMPLE(toSample, fGeom.m_UV1 + distortionUVs).xyz;
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec2	SampleTextureVec2(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	vec2	color = SAMPLEGRAD(toSample, fGeom.m_UV0, fGeom.m_dUVdx, fGeom.m_dUVdy).xy;
#else
	vec2	color = SAMPLE(toSample, fGeom.m_UV0).xy;
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec2	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1, fGeom.m_dUVdx, fGeom.m_dUVdy).xy;
#else
		vec2	color1 =  SAMPLE(toSample, fGeom.m_UV1).xy;
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

float	SampleTextureVec1(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
#if defined(HAS_TransformUVs)
	float	color = SAMPLEGRAD(toSample, fGeom.m_UV0, fGeom.m_dUVdx, fGeom.m_dUVdy).x;
#else
	float	color = SAMPLE(toSample, fGeom.m_UV0).x;
#endif
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		float	color1 =  SAMPLEGRAD(toSample, fGeom.m_UV1, fGeom.m_dUVdx, fGeom.m_dUVdy).x;
#else
		float	color1 =  SAMPLE(toSample, fGeom.m_UV1).x;
#endif
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

#if	defined(HAS_TransformUVs)
float	SampleTextureAlpha(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
	float	color = SAMPLEGRAD(toSample, fGeom.m_AlphaUV0, fGeom.m_dUVdx, fGeom.m_dUVdy).w;
#	if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		float	color1 =  SAMPLEGRAD(toSample, fGeom.m_AlphaUV1, fGeom.m_dUVdx, fGeom.m_dUVdy).w;
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#	endif
	return color;
}
#endif
