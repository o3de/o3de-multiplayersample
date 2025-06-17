#pragma once

#include "PKSurface.h"

#if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)

#	if defined(HAS_Emissive)

#		if defined(HAS_UVDistortions)
void	ApplyEmissive(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, IN(SMaskDistortion) fMaskDistortion, vec4 emissiveColor FS_ARGS)
#		else
void	ApplyEmissive(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec4 emissiveColor FS_ARGS)
#		endif
{
#		if defined(HAS_UVDistortions)
	vec2 distortion = fMaskDistortion.m_UVDistortion1 * 0.1 + fMaskDistortion.m_UVDistortion2 * 0.01;
	vec4	textureColor = SampleDistortedTextureVec4(fGeom, distortion, SAMPLER_ARG(Emissive_EmissiveMap) FS_PARAMS);
		#else
	vec4	textureColor = SampleTextureVec4(fGeom, SAMPLER_ARG(Emissive_EmissiveMap) FS_PARAMS);
#		endif

#		if	defined(HAS_TransformUVs)
	if (fGeom.m_UseAlphaUVs)
		textureColor.a = SampleTextureAlpha(fGeom, SAMPLER_ARG(Emissive_EmissiveMap) FS_PARAMS);
#		endif
	fSurf.m_Emissive = (emissiveColor.rgb * textureColor.rgb) * (emissiveColor.a * textureColor.a);
}
#	endif

#	if defined(HAS_EmissiveRamp)
void	ApplyEmissiveRamp(IN(float) v, INOUT(SFragSurface) fSurf FS_ARGS)
{
	vec2 uv = vec2(fSurf.m_Emissive.r, v);
	fSurf.m_Emissive = SAMPLE(EmissiveRamp_RampMap, uv).rgb;
}
#	endif
#endif
