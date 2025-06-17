#pragma once

#include "PKSurface.h"

#if defined(HAS_SoftParticles)

float	ClipToLinearDepth(float depth FS_ARGS)
{
	float	zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float	zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	return (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
}

void	ApplySoftParticles(INOUT(SFragSurface) fSurf, vec3 clipPos FS_ARGS)
{
	vec2	screenUV = clipPos.xy * vec2(0.5, 0.5) + 0.5;
	float	clipSceneDepth = SAMPLE(DepthSampler, screenUV).x;
	float	sceneDepth = ClipToLinearDepth(clipSceneDepth FS_PARAMS);
	float	fragDepth = ClipToLinearDepth(clipPos.z FS_PARAMS);
	float	invSoftnessDistance = 1.0f / GET_CONSTANT(Material, SoftParticles_SoftnessDistance);
	float	depthFade = clamp((sceneDepth - fragDepth) * invSoftnessDistance, 0.f, 1.f);
#if defined(PK_FORWARD_DISTORTION_PASS)
	fSurf.m_Distortion *= vec3(depthFade, depthFade, depthFade);
#elif defined(PK_FORWARD_TINT_PASS)
	fSurf.m_Tint = mix(VEC3_ONE, fSurf.m_Tint, depthFade);
#elif defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
	fSurf.m_Emissive *= vec3(depthFade, depthFade, depthFade);
	fSurf.m_Diffuse.a *= depthFade;
#else
#	error "Unrecognized particle render pass"
#endif
}

#endif
