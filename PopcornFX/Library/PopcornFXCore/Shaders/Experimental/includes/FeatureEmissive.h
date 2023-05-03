#pragma once

#include "PKSurface.h"

#if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)

#	if defined(HAS_Emissive)
void	ApplyEmissive(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec3 emissiveColor FS_ARGS)
{
	vec4	textureColor = SampleTextureVec4(fGeom, SAMPLER_ARG(Emissive_EmissiveMap) FS_PARAMS);
#		if	defined(HAS_TransformUVs)
	if (fGeom.m_UseAlphaUVs)
		textureColor.a = SampleTextureAlpha(fGeom, SAMPLER_ARG(Emissive_EmissiveMap) FS_PARAMS);
#		endif
	fSurf.m_Emissive = emissiveColor * textureColor.rgb * textureColor.a;
}
#	endif

#	if defined(HAS_EmissiveRamp)
void	ApplyEmissiveRamp(INOUT(SFragSurface) fSurf FS_ARGS)
{
	fSurf.m_Emissive = SAMPLE(EmissiveRamp_RampMap, vec2(fSurf.m_Emissive.r, 0.0)).rgb;
}
#	endif

#endif
