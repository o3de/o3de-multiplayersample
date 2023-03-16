#pragma once

#include "PKSurface.h"

#if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)

#	if defined(HAS_Diffuse)

#		if	defined(HAS_AlphaRemap)
void		ApplyAlphaRemap(INOUT(SFragSurface) fSurf, float cursor FS_ARGS)
{
	vec2	alphaTexCoord = vec2(fSurf.m_Diffuse.a, cursor);
	fSurf.m_Diffuse.a = SAMPLE(AlphaRemap_AlphaMap, alphaTexCoord).r;
}
#		endif // defined(HAS_AlphaRemap)

#		if	defined(HAS_AlphaRemap)
void		ApplyDiffuse(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec4 diffuseColor, float alphaRemapCursor FS_ARGS)
#		else
void		ApplyDiffuse(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec4 diffuseColor FS_ARGS)
#		endif // defined(HAS_AlphaRemap)
{
	fSurf.m_Diffuse = SampleTextureVec4(fGeom, SAMPLER_ARG(Diffuse_DiffuseMap) FS_PARAMS);
#		if	defined(HAS_TransformUVs)
	if (fGeom.m_UseAlphaUVs)
		fSurf.m_Diffuse.a = SampleTextureAlpha(fGeom, SAMPLER_ARG(Diffuse_DiffuseMap) FS_PARAMS);
#		endif // defined(HAS_TransformUVs)

#		if	defined(HAS_AlphaRemap)
	ApplyAlphaRemap(fSurf, alphaRemapCursor FS_ARGS);
#		endif
	fSurf.m_Diffuse *= diffuseColor;

	// We clamp the alpha between 0 and 1 now:
	fSurf.m_Diffuse.a = clamp(fSurf.m_Diffuse.a, 0.0f, 1.0f);
}
#	endif // defined(HAS_Diffuse)

#	if defined(HAS_DiffuseRamp)
void		ApplyDiffuseRamp(INOUT(SFragSurface) fSurf FS_ARGS)
{
	fSurf.m_Diffuse.rgb = SAMPLE(DiffuseRamp_RampMap, vec2(fSurf.m_Diffuse.r, 0.0)).rgb;
}
#	endif // defined(HAS_DiffuseRamp)

#endif // defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)
