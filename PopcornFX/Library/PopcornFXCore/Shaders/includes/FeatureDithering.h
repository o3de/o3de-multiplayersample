#pragma once

#include "PKSurface.h"

#if defined(PK_DEFERRED_COLOR_PASS)
void	ApplyDithering(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, IN(SFragmentInput) fInput FS_ARGS)
{
	ivec2	iTexDim;
	TEXTURE_DIMENSIONS(DitheringPatterns, 0, iTexDim)
	vec2	texDim = CAST(vec2, iTexDim);

	float	cellCount = 17.0;
	vec2	cellStart = vec2(floor(fSurf.m_Diffuse.a * cellCount) / cellCount, 0);
	vec2	cellDim = vec2(texDim.x / cellCount, texDim.y);

	vec2 	viewportSize = GET_CONSTANT(SceneInfo, ViewportSize);
	vec3 	clipPos = fInput.fragViewProjPosition.xyz / fInput.fragViewProjPosition.w;

	vec2	screenUV = clipPos.xy * vec2(0.5, 0.5) + 0.5;
	screenUV = screenUV * viewportSize;
	screenUV = mod(screenUV, cellDim);
	screenUV = screenUV / cellDim;
	screenUV = fract(screenUV + fInput.fragOpaque_DitheringOffset);

	screenUV.x = (cellStart.x + screenUV.x * cellDim.x / texDim.x);

	float	noise =  SAMPLE(DitheringPatterns, screenUV).r;
	int		ditheringMode = GET_CONSTANT(Material, Opaque_DitheringMode);

	// Note: noise is actually either 0 or 1.
	if (ditheringMode == 0 && noise < 0.5) // Regular
		discard;
	else if (ditheringMode == 1 && noise > 0.5) // InversedAlpha
		discard;
}
#endif
