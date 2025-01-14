#pragma once

#include "PKSurface.h"

vec2	TransformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}

#if	defined(HAS_TransformUVs)

#	if defined(HAS_Atlas)
void	ApplyTransformUVs(INOUT(SFragGeometry) fGeom, vec4 rect0, vec4 rect1, float angle, vec2 scale, vec2 offset FS_ARGS)
#else
void	ApplyTransformUVs(INOUT(SFragGeometry) fGeom, float angle, vec2 scale, vec2 offset FS_ARGS)
#endif
{
	float	sinR = sin(angle);
	float	cosR = cos(angle);
	mat2	UVRotation = mat2(cosR, sinR, -sinR, cosR);
	vec2	rotationCenter = GET_CONSTANT(Material, TransformUVs_RotationCenter);

#	if defined(HAS_Atlas)
	fGeom.m_AlphaUV1 = fGeom.m_UV1;
	fGeom.m_UV1 = ((fGeom.m_UV1 - rect1.zw) / rect1.xy); // normalize (if atlas)
	fGeom.m_UV1 = TransformUV(fGeom.m_UV1 - rotationCenter, scale, UVRotation, offset + scale*rotationCenter); // scale then rotate then translate UV
	fGeom.m_UV1 = fract(fGeom.m_UV1) * rect1.xy + rect1.zw; // undo normalize
#	else
	vec4	rect0 = vec4(1.0f, 1.0f, 0.0f, 0.0f);
#	endif

	fGeom.m_AlphaUV0 = fGeom.m_UV0;
	fGeom.m_UV0 = ((fGeom.m_UV0 - rect0.zw) / rect0.xy); // normalize (if atlas)
	fGeom.m_UV0 = TransformUV(fGeom.m_UV0 - rotationCenter, scale, UVRotation, offset + scale*rotationCenter); // scale then rotate then translate UV
	// Compute clean derivatives
	vec2	_uv = fGeom.m_UV0 * rect0.xy + rect0.zw;
	fGeom.m_dUVdx = dFdx(_uv);
	fGeom.m_dUVdy = dFdy(_uv);
	fGeom.m_UV0 = fract(fGeom.m_UV0) * rect0.xy + rect0.zw; // undo normalize

	fGeom.m_UseAlphaUVs = GET_CONSTANT(Material, TransformUVs_RGBOnly) != 0;

	float	sinMR = -sinR;
	float	cosMR = cosR;
	fGeom.m_TangentRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
}

#endif

#if	defined(HAS_BasicTransformUVs)

void	ApplyBasicTransformUVs(INOUT(SFragGeometry) fGeom FS_ARGS)
{
#	if !defined(HAS_GeometryRibbon)
#		if defined(HAS_GeometryDecal)
		bool flipU = bool(GET_CONSTANT(Material, BasicTransformUVs_FlipU));
		bool flipV = bool(GET_CONSTANT(Material, BasicTransformUVs_FlipV));
		
		if (flipU)
		{
			fGeom.m_UV0.x = 1.0f - fGeom.m_UV0.x;
#			if defined(HAS_Atlas)
			fGeom.m_UV1.x = 1.0f - fGeom.m_UV1.x;
#			endif
		}

		if (flipV)
		{
			fGeom.m_UV0.y = 1.0f - fGeom.m_UV0.y;
#			if defined(HAS_Atlas)
			fGeom.m_UV1.y = 1.0f - fGeom.m_UV1.y;
#			endif
		}
#		endif

	if (GET_CONSTANT(Material, BasicTransformUVs_RotateUV) != 0)
	{
		fGeom.m_UV0 = fGeom.m_UV0.yx;
#		if defined(HAS_Atlas)
		fGeom.m_UV1 = fGeom.m_UV1.yx;
#		endif // defined(HAS_Atlas)
	}
#	endif // !defined(HAS_GeometryRibbon)
}

#endif
