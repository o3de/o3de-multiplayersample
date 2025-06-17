
#include "includes/PKLighting.h"
#include "includes/PKOutputColor.h"

//------------------------------
// BRDF Computation
//------------------------------
#define PI                      3.141592f
#define METAL_FRESNEL_FACTOR    vec3(0.04f, 0.04f, 0.04f)
#define EPSILON                 1.0e-5f

void        FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec3 			clipPos = fInput.fragViewProjPosition.xyz / fInput.fragViewProjPosition.w;
	SFragSurface 	fSurf;

	GBufferToFragSurface(fSurf, clipPos FS_PARAMS);

	float	lightRange = fInput.fragLightAttenuation_Range;
	vec3	lightColor = fInput.fragLight_Color.xyz;
	vec3	lightPosition = fInput.fragLightPosition;

	ApplyPointLight(fSurf, lightColor, lightPosition, lightRange FS_PARAMS);

	OutputFragmentColor(fSurf, fOutput FS_PARAMS);
}
