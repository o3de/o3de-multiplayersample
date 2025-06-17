
#include "includes/FeatureRibbonCorrectDeformation.h"
#include "includes/FeatureAtlas.h"
#include "includes/FeatureTransformUVs.h"
#include "includes/FeatureDistortion.h"
#include "includes/FeatureTint.h"
#include "includes/FeatureDiffuse.h"
#include "includes/FeatureEmissive.h"
#include "includes/FeatureSoftParticles.h"
#include "includes/FeatureDecal.h"
#include "includes/FeatureDithering.h"
#include "includes/FeatureAlphaMasks.h"
#include "includes/FeatureUVDistortions.h"
#include "includes/FeatureFastLit.h"
#include "includes/PKLighting.h"
#include "includes/PKOutputColor.h"

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec3 			clipPos = fInput.fragViewProjPosition.xyz / fInput.fragViewProjPosition.w;

	// -------------------
	// Frag geom:
	// -------------------
	SFragGeometry 	fGeom;
	FragmentInputToFragGeometry(fInput, fGeom FS_PARAMS);

#if defined(HAS_AlphaMasks) || defined(HAS_UVDistortions)
	SMaskDistortion fMaskDist;
	InitializeMaskDistortion(fMaskDist);
#endif

#if defined(HAS_CorrectDeformation)
	ApplyRibbonCorrectDeformation(fGeom, fInput.fragUVFactors, fInput.fragUVScaleAndOffset FS_PARAMS);
#endif

#if defined(HAS_UVDistortions)
	ApplyUVDistortion(fGeom, fInput.fragUVDistortions_Distortion1AnimationCursor, fInput.fragUVDistortions_Distortion2AnimationCursor, fMaskDist);
#endif

#if defined(HAS_AlphaMasks)
	ApplyAlphaMasks(fGeom, fInput.fragAlphaMasks_Mask1AnimationCursor, fInput.fragAlphaMasks_Mask2AnimationCursor, fMaskDist FS_PARAMS);
#endif

#if defined(HAS_Atlas)
	ApplyAtlasTexCoords(fGeom FS_PARAMS);
#endif

#if	defined(HAS_TransformUVs)
#	if defined(HAS_Atlas)
	uint	maxAtlasID = LOADU(GET_RAW_BUFFER(Atlas), 0) - 1;
	vec4	rect0 = LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(maxAtlasID, uint(fInput.fragAtlas_TextureID)) * 4 + 1));
	vec4	rect1 = LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(maxAtlasID, uint(fInput.fragAtlas_TextureID) + 1) * 4 + 1));
	ApplyTransformUVs(fGeom, rect0, rect1, fInput.fragTransformUVs_UVRotate, fInput.fragTransformUVs_UVScale, fInput.fragTransformUVs_UVOffset FS_PARAMS);
#	else
	ApplyTransformUVs(fGeom, fInput.fragTransformUVs_UVRotate, fInput.fragTransformUVs_UVScale, fInput.fragTransformUVs_UVOffset FS_PARAMS);
#	endif
#endif
#	if	defined(HAS_BasicTransformUVs)
	ApplyBasicTransformUVs(fGeom FS_PARAMS);
#	endif

#	if defined(HAS_Lit) && !defined(HAS_FastLit)
#		if(HAS_UVDistortions)
	ApplyNormalMap(fGeom, fMaskDist, fInput.IsFrontFace FS_PARAMS);
#		else
	ApplyNormalMap(fGeom, fInput.IsFrontFace FS_PARAMS);
#		endif
#	endif

	// -------------------
	// Frag surface:
	// -------------------
	SFragSurface 	fSurf;
	FragGeometryToFragSurface(fGeom, fSurf, clipPos.z FS_PARAMS);

#if 	defined(PK_FORWARD_DISTORTION_PASS)

	// Distortion render pass:
	ApplyDistortion(fSurf, fGeom, fInput.fragDistortion_DistortionColor, clipPos.z FS_PARAMS);

#elif 	defined(PK_FORWARD_TINT_PASS)

	// Tint render pass:
	ApplyTint(fSurf, fGeom, fInput.fragTint_TintColor FS_PARAMS);

#elif 	defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS) || defined(PK_DEFERRED_DECAL_PASS)

	// Color render pass:
#	if	defined(HAS_Diffuse)
#		if	defined(HAS_AlphaRemap)
	ApplyDiffuse(fSurf, fGeom, fInput.fragDiffuse_DiffuseColor, fInput.fragAlphaRemap_AlphaRemapCursor FS_PARAMS);
#		elif defined(HAS_UVDistortions)
	ApplyDiffuse(fSurf, fGeom, fMaskDist, fInput.fragDiffuse_DiffuseColor FS_PARAMS);
#		else
	ApplyDiffuse(fSurf, fGeom, fInput.fragDiffuse_DiffuseColor FS_PARAMS);
#		endif
#	endif

#	if	defined(HAS_Emissive)
#		if defined(HAS_UVDistortions)
	ApplyEmissive(fSurf, fGeom, fMaskDist, fInput.fragEmissive_EmissiveColor FS_PARAMS);
#		else
	ApplyEmissive(fSurf, fGeom, fInput.fragEmissive_EmissiveColor FS_PARAMS);
#		endif
#	endif

#	if defined(HAS_AlphaMasks) || defined(HAS_UVDistortions)
	ApplyFinalAlpha(fInput, fGeom, fSurf, fMaskDist FS_PARAMS);
#	else
	ApplyFinalAlpha(fInput, fGeom, fSurf FS_PARAMS);
#	endif

#	if	defined(HAS_EmissiveRamp)
		ApplyEmissiveRamp(fInput.fragEmissiveRamp_RampVAxis, fSurf FS_PARAMS);
#	endif

#	if	defined(HAS_DiffuseRamp)
	ApplyDiffuseRamp(fInput.fragDiffuseRamp_RampVAxis, fSurf FS_PARAMS);
#	endif

#   if defined(HAS_Dissolve)
	ApplyDissolve(fInput.fragDissolve_DissolveAnimationCursor, fGeom, fSurf FS_PARAMS);
#   endif

#	if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
#		if	defined(HAS_Lit) && !defined(HAS_FastLit)
	float 	roughness = GET_CONSTANT(Material, Lit_Roughness);
	float	metalness = GET_CONSTANT(Material, Lit_Metalness);
	ApplyLighting(fSurf, fGeom, fInput.fragWorldPosition, roughness, metalness FS_PARAMS);
#		endif

#		if	defined(HAS_FastLit)
	ApplyFastLighting(fSurf, fGeom, fInput.fragWorldPosition FS_PARAMS);
#		endif
#	endif // !defined(PK_DEFERRED_DECAL_PASS)


#	if defined(PK_DEFERRED_COLOR_PASS)
	if (GET_CONSTANT(Material, Opaque_Type) == 2)
		ApplyDithering(fSurf, fGeom, fInput FS_PARAMS);
#	endif

#else
#	error "Unrecognized particle render pass"
#endif

#if defined(PK_DEFERRED_DECAL_PASS)
	ApplyDecalFading(fSurf, fGeom FS_PARAMS);
#elif !defined(PK_DEFERRED_COLOR_PASS)
# 	if defined(HAS_SoftParticles)
	// For all render passes except deferred :
	ApplySoftParticles(fSurf, clipPos FS_PARAMS);
# 	endif
#endif

	OutputFragmentColor(fSurf, fOutput FS_PARAMS);
}

