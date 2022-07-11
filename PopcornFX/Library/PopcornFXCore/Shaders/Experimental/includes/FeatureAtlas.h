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
	vec4	color = SAMPLE(toSample, fGeom.m_UV0);
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		vec4	color1 =  SAMPLE(toSample, fGeom.m_UV1);
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec3	SampleTextureVec3(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
	vec3	color = SAMPLE(toSample, fGeom.m_UV0).xyz;
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		vec3	color1 =  SAMPLE(toSample, fGeom.m_UV1).xyz;
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

vec2	SampleTextureVec2(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
	vec2	color = SAMPLE(toSample, fGeom.m_UV0).xy;
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		vec2	color1 =  SAMPLE(toSample, fGeom.m_UV1).xy;
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

float	SampleTextureVec1(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
	float	color = SAMPLE(toSample, fGeom.m_UV0).x;
#if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		float	color1 =  SAMPLE(toSample, fGeom.m_UV1).x;
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#endif
	return color;
}

#if	defined(HAS_TransformUVs)
float	SampleTextureAlpha(IN(SFragGeometry) fGeom, SAMPLER2D_DCL_ARG(toSample) FS_ARGS)
{
	float	color = SAMPLE(toSample, fGeom.m_AlphaUV0).w;
#	if defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	if (blendingType >= 1)
	{
		float	color1 =  SAMPLE(toSample, fGeom.m_AlphaUV1).w;
		color = mix(color, color1, fGeom.m_BlendMix);
	}
#	endif
	return color;
}
#endif
