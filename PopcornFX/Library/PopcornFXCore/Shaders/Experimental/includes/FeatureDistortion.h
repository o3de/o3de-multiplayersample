#pragma once

#include "PKSurface.h"

#if defined(ParticlePass_Distortion)

float	ClipDepthToDistortionFade(float depth FS_ARGS)
{
	float	zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float	zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	float	zLinear = (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
	return min(zLinear / zNear - 1.f, sqrt(10.f / zLinear) ); // looks like the v1 (10.f = sqrt(zFar * zNear))
}

void	ApplyDistortion(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec3 distortionColor, float clipDepth FS_ARGS)
{
	// 128 is considered as the mid-value (in range 0-255):
	vec3	textureColor = SampleTextureVec3(fGeom, SAMPLER_ARG(Distortion_DistortionMap) FS_PARAMS) * vec3(2.0f, 2.0f, 1.0f) - vec3(1.00392f, 1.00392f, 0.0f);
	fSurf.m_Distortion = distortionColor * textureColor;
	fSurf.m_Distortion *= ClipDepthToDistortionFade(clipDepth FS_PARAMS);
}

#endif
