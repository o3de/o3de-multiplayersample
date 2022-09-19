#pragma once

#include "PKSurface.h"

#if defined(HAS_GeometryDecal)

void	ApplyDecalFading(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom FS_ARGS)
{
	float	fadeRangeTop = GET_CONSTANT(Material, Decal_FadeTop);
	float	startFadeTop = 1.0f - fadeRangeTop;
	float	fadeRangeBottom = GET_CONSTANT(Material, Decal_FadeBottom);
	float	startFadeBottom = 1.0f - fadeRangeBottom;

	float	startFade = (fGeom.m_NormalizedDecalSpace.z < 0.0f) ? startFadeBottom : startFadeTop;
	float	fadeRange = (fGeom.m_NormalizedDecalSpace.z < 0.0f) ? fadeRangeBottom : fadeRangeTop;
	float	fade = 1.0f - clamp((abs(fGeom.m_NormalizedDecalSpace.z) - startFade) / fadeRange, 0.0f, 1.0f);

	fSurf.m_Diffuse.a *= fade;
	fSurf.m_Emissive.rgb *= fade;
}

#endif
