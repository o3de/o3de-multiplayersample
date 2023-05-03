#pragma once

#include "PKSurface.h"

#if defined(HAS_CorrectDeformation)

void	ApplyRibbonCorrectDeformation(INOUT(SFragGeometry) fGeom, vec4 fragUVFactors, vec4 fragUVScaleAndOffset FS_ARGS)
{
#	if defined(FINPUT_fragUV0) // Particle has UV0

	vec2	fragUVout;
	if (fGeom.m_UV0.x + fGeom.m_UV0.y < 1)
		fragUVout = fGeom.m_UV0.xy / fragUVFactors.xy;
	else
		fragUVout = 1.0 - ((1.0 - fGeom.m_UV0.xy) / fragUVFactors.zw);

#		if defined(HAS_BasicTransformUVs)
	if (GET_CONSTANT(Material, BasicTransformUVs_RotateUV) != 0)
		fragUVout = fragUVout.yx;
#		endif

	fGeom.m_UV0 = fragUVout * fragUVScaleAndOffset.xy + fragUVScaleAndOffset.zw;

#	endif
#	if defined(FINPUT_fragUV1)
#		error "Ribbon correct deformation is not compatible with atlas"
#	endif
}

#endif
