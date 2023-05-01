#pragma once

#include "PKSurface.h"

#if defined(PK_FORWARD_TINT_PASS)

#	if defined(HAS_Tint)
void	ApplyTint(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec4 tintColor FS_ARGS)
{
	vec4	textureColor = SampleTextureVec4(fGeom, SAMPLER_ARG(Tint_TintMap) FS_PARAMS);
#		if	defined(HAS_TransformUVs)
	if (fGeom.m_UseAlphaUVs)
		textureColor.a = SampleTextureAlpha(fGeom, SAMPLER_ARG(Tint_TintMap) FS_PARAMS);
#		endif
	vec4	tintColorWithAlpha = tintColor * textureColor;
	tintColorWithAlpha.a = clamp(tintColorWithAlpha.a, 0.0f, 1.0f);
	fSurf.m_Tint = mix(vec3(1.0f, 1.0f, 1.0f), tintColorWithAlpha.rgb, tintColorWithAlpha.a);
}
#	endif

#endif
