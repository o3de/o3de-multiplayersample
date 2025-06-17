#pragma once

#include "PKSurface.h"

#if defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)
vec2	PackNormalSpheremap(vec3 normal FS_ARGS)
{
	if (ALL_BOOL(VEC_EQ(normal, VEC3_ZERO)))
		return VEC2_ZERO;
	normal = normalize(normal);
	normal = mul(GET_CONSTANT(SceneInfo, PackNormalView), vec4(normal, 0.0f)).xyz;
	float	f = sqrt(8.0f * normal.z + 8.0f);
	return normal.xy / f + 0.5f;
}
#endif

void	OutputFragmentColor(IN(SFragSurface) fSurf, OUT(SFragmentOutput) fOutput FS_ARGS)
{
#if defined(PK_FORWARD_DISTORTION_PASS)
	fOutput.Output0 = vec4(fSurf.m_Distortion, 1.0f);
#elif defined(PK_FORWARD_TINT_PASS)
	fOutput.Output0 = vec4(fSurf.m_Tint, 1.0f);
#elif defined(PK_FORWARD_COLOR_PASS)
	fOutput.Output0 = vec4(fSurf.m_Emissive + fSurf.m_Diffuse.rgb * fSurf.m_Diffuse.a, fSurf.m_Diffuse.a);
#elif defined(PK_DEFERRED_COLOR_PASS)

	// Discard pixel if masked:
	if (GET_CONSTANT(Material, Opaque_Type) == 1 && fSurf.m_Diffuse.a < GET_CONSTANT(Material, Opaque_MaskThreshold))
		discard;

#	if defined(ParticlePass_OpaqueShadow)
	fOutput.Output0 = vec2(fSurf.m_Depth, fSurf.m_Depth * fSurf.m_Depth);
#	else
	// In the case the of unlit opaque, we transfer the diffuse color to the emissve buffer:
#		if !(defined(HAS_Lit) || defined(HAS_FastLit))
	fSurf.m_Emissive += fSurf.m_Diffuse.xyz;
	fSurf.m_Diffuse = VEC4_ZERO;
	fSurf.m_Roughness = -1.0f;
	fSurf.m_Metalness = -1.0f;
#		endif
	fOutput.Output0 = fSurf.m_Diffuse;
	fOutput.Output1 = fSurf.m_Depth;
	fOutput.Output2 = vec4(fSurf.m_Emissive, 0.0f);
	fOutput.Output3 = vec4(PackNormalSpheremap(fSurf.m_Normal FS_PARAMS), fSurf.m_Roughness, fSurf.m_Metalness);
#	endif // !defined(ParticlePass_OpaqueShadow)

#elif defined(PK_DEFERRED_DECAL_PASS)
	fOutput.Output0 = vec4(fSurf.m_Diffuse.rgb * fSurf.m_Diffuse.a, fSurf.m_Diffuse.a);
	fOutput.Output1 = vec4(fSurf.m_Emissive, 0.0f);
	fOutput.Output2 = vec4(PackNormalSpheremap(fSurf.m_Normal FS_PARAMS), fSurf.m_Roughness, fSurf.m_Metalness);
#elif defined(PK_LIGHTING_PASS)
	fOutput.Output0 = vec4(fSurf.m_LightAccu, 0.0f);
#else
#	error "Unrecognized particle render pass"
#endif
}
