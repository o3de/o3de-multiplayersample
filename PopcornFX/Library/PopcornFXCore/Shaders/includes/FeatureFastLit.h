#pragma once

#include "PKSurface.h"

#if (defined(PK_FORWARD_COLOR_PASS) && defined(HAS_FastLit)) || defined(PK_LIGHTING_PASS)

#	define PI						3.141592
#	define METAL_FRESNEL_FACTOR		vec3(0.04, 0.04, 0.04) // F0
#	define EPSILON					1e-5

void	ApplyFastLightBRDF(INOUT(SFragSurface) fSurf, vec3 surfToLight, vec3 lightColor, vec3 surfaceDiffuseColor, float attenuation FS_ARGS)
{
	float   NoL = dot(surfToLight, fSurf.m_Normal);

#if     defined(HAS_NormalWrap)
	float   normalWrapFactor = GET_CONSTANT(Material, NormalWrap_WrapFactor) * 0.5f;
	NoL = normalWrapFactor + (1.0f - normalWrapFactor) * NoL;
#endif

	NoL = max(0.0f, NoL) * attenuation;

#	if defined(PK_LIGHTING_PASS)
	fSurf.m_LightAccu += surfaceDiffuseColor * lightColor * NoL;
#	else
    fSurf.m_Diffuse.rgb += surfaceDiffuseColor * lightColor * NoL;
#	endif
}

#   if defined(PK_FORWARD_COLOR_PASS)

vec3	toCubemapSpace(vec3 value FS_ARGS)
{
	vec3	valueLHZ = mul(GET_CONSTANT(SceneInfo, UserToLHZ), vec4(value, 0.0f)).xyz;
	vec3	valueRHY = vec3(valueLHZ.x, valueLHZ.z, valueLHZ.y);
	valueRHY.xz = mul(GET_CONSTANT(EnvironmentMapInfo, Rotation), valueRHY.xz);
	return valueRHY;
}

void	ApplyFastAmbientBRDF(INOUT(SFragSurface) fSurf, vec3 surfToView, vec3 ambientColor, vec3 surfaceDiffuseColor FS_ARGS)
{
	// Cubemaps are expected to be cosined filtered up to the 7th mipmap which should contain the fully diffuse 180 degrees cosine-filtered cubemap.
	const float	kCubemapMipRange = 6;
    fSurf.m_Roughness = 1.0f;

	const vec3	surfaceMetalColor = METAL_FRESNEL_FACTOR; // mix(METAL_FRESNEL_FACTOR, surfaceDiffuseColor, fSurf.m_Metalness); // TODO
	const vec3	kD = 1.0f;

	// Ambient diffuse
	vec3	diffuseBRDF = kD * SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(fSurf.m_Normal FS_PARAMS), kCubemapMipRange).rgb;
	// Ambient specular
	vec3	sampleDir = toCubemapSpace(reflect(-surfToView, fSurf.m_Normal) FS_PARAMS);
	vec3	prefilteredColor = SAMPLELOD_CUBE(EnvironmentMapSampler, sampleDir, kCubemapMipRange).rgb;
	vec2	envBRDF = SAMPLE(BRDFLUTSampler, vec2(max(dot(fSurf.m_Normal, surfToView), 0.0), 1.0f)).rg;
	vec3	specularBRDF = prefilteredColor * envBRDF.y;

#		if defined(HAS_Transparent)
	vec3	litDiffuse = surfaceDiffuseColor * diffuseBRDF * ambientColor;
	// Mix between the computed lit color and the original surface color
	// Completly overrides the previous computed lit color:
	fSurf.m_Diffuse.rgb = litDiffuse;
	// We write the specular in the emissive output so that it doesn't get affected by the diffuse alpha:
	fSurf.m_Emissive += specularBRDF * ambientColor;
#		elif defined(HAS_Opaque)
	fSurf.m_Diffuse.rgb += surfaceDiffuseColor * (diffuseBRDF + specularBRDF) * ambientColor;
#		endif // defined(HAS_Transparent)
}
#   endif // defined(PK_FORWARD_COLOR_PASS)
#endif // (defined(PK_FORWARD_COLOR_PASS) && defined(HAS_Lit)) || defined(PK_LIGHTING_PASS)

#if defined(HAS_FastLit)
float 		remapValue(float value, float oldMin, float oldMax, float newMin, float newMax)
{
	return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
}

void ApplyFastLighting(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec3 surfacePosition FS_ARGS)
{
#	if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
	fSurf.m_Roughness = 1.0f;
	fSurf.m_Metalness = 0.0f;
#	endif
#	if defined(PK_FORWARD_COLOR_PASS)

	vec3	viewPosition = GET_MATRIX_W_AXIS(GET_CONSTANT(SceneInfo, UnpackNormalView)).xyz;
	vec3	surfToView = viewPosition - surfacePosition;
	vec3	surfToViewDir = normalize(surfToView);
	// Before lighting, we backup the surface diffuse color and reset the diffuse to 0 to accumulate the light influences:
	vec3	surfaceDiffuseColor = fSurf.m_Diffuse.rgb;
	fSurf.m_Diffuse.rgb = VEC3_ZERO;

	vec3 	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);

	ApplyFastAmbientBRDF(fSurf, surfToViewDir, ambientColor, surfaceDiffuseColor FS_PARAMS); //

	int	lightsDirectional = GET_CONSTANT(LightInfo, DirectionalLightsCount);
	int	lightsSpot = GET_CONSTANT(LightInfo, SpotLightsCount);
	int	lightsPoint = GET_CONSTANT(LightInfo, PointLightsCount);

	// 3 x float4 per light:
	const int 	lightStride = 3 * 4;

	for (int dirIdx = 0; dirIdx < lightsDirectional; ++dirIdx)
	{
		// Directional layout is:
		// float3 direction
		// float padding
		// float4 padding
		// float3 color
		// float padding
		vec3	direction = LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(dirIdx * lightStride));
		vec3	color = LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(dirIdx * lightStride + 8));

		// Compute lighting:
		vec3	surfToLightDir = normalize(-direction);
		ApplyFastLightBRDF(fSurf, surfToLightDir, color, surfaceDiffuseColor, 1.0f FS_ARGS);
	}
	for (int spotIdx = 0; spotIdx < lightsSpot; ++spotIdx)
	{
		// Spot layout is:
		// float3 position
		// float angle
		// float3 direction
		// float cone falloff
		// float3 color
		// float padding
		vec4	positionAngle = LOADF4(GET_RAW_BUFFER(SpotLightsInfo), RAW_BUFFER_INDEX(spotIdx * lightStride));
		vec4	directionFalloff = LOADF4(GET_RAW_BUFFER(SpotLightsInfo), RAW_BUFFER_INDEX(spotIdx * lightStride + 4));
		vec3	color = LOADF3(GET_RAW_BUFFER(SpotLightsInfo), RAW_BUFFER_INDEX(spotIdx * lightStride + 8));

		vec3	surfToLight = positionAngle.xyz - surfacePosition;
		vec3	surfToLightDir = normalize(surfToLight);
		float	sqDistanceToLight = dot(surfToLight, surfToLight);
	
		// Compute the light attenuation
		// Start by fading depending on the cone angle:
		float   coneAttenuation = dot(-surfToLightDir, directionFalloff.xyz);
		coneAttenuation = coneAttenuation < positionAngle.w ? 0.0f : remapValue(coneAttenuation, positionAngle.w, 1.0f, 0.0f, 1.0f);
		coneAttenuation = min(coneAttenuation / max(directionFalloff.w, EPSILON), 1.0f);
		float   lightAttenuation = coneAttenuation / sqDistanceToLight;

		ApplyFastLightBRDF(fSurf, surfToLightDir, color, surfaceDiffuseColor, lightAttenuation FS_ARGS);
	}
	for (int pointIdx = 0; pointIdx < lightsPoint; ++pointIdx)
	{
		// Point layout is:
		// float3 position
		// float padding
		// float4 padding
		// float3 color
		// float padding
		vec3	position = LOADF3(GET_RAW_BUFFER(PointLightsInfo), RAW_BUFFER_INDEX(pointIdx * lightStride));
		vec3	color = LOADF3(GET_RAW_BUFFER(PointLightsInfo), RAW_BUFFER_INDEX(pointIdx * lightStride + 8));

		// Compute lighting:
		vec3	surfToLight = position - surfacePosition;
		float	sqDistanceToLight = dot(surfToLight, surfToLight);
		vec3	surfToLightDir = normalize(surfToLight);
		// (here we can use the square distance falloff as this is rendered as a full screen quad and there is no culling):
		float   lightAttenuation = 1.0f / sqDistanceToLight;

		ApplyFastLightBRDF(fSurf, surfToLightDir, color, surfaceDiffuseColor, lightAttenuation FS_ARGS);
	}
#	endif // defined(PK_FORWARD_COLOR_PASS)
}
#endif // defined(HAS_Lit)
