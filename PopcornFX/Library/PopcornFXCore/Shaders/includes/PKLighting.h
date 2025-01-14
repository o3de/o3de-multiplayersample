#pragma once

#include "PKSurface.h"
#include "FeatureAtlas.h"

#if (defined(PK_FORWARD_COLOR_PASS) && defined(HAS_Lit)) || defined(PK_LIGHTING_PASS)

//------------------------------
// BRDF Computation
//------------------------------
#	define PI						3.141592
#	define METAL_FRESNEL_FACTOR		vec3(0.04, 0.04, 0.04) // F0
#	define EPSILON					1e-5

float	NormalDistFuncGGX(float cosLh, float roughness)
{
	float	alpha = roughness * roughness;
	float	alphaSq = alpha * alpha;
	float	denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float	SchlickGGX(float cosLi, float cosLo, float roughness)
{
	float	r = roughness + 1.0;
	float	k = (r * r) / 8.0; // UE4 remapping
	float	t1 = cosLi / (cosLi * (1.0 - k) + k);
	float	t2 = cosLo / (cosLo * (1.0 - k) + k);
	return t1 * t2;
}

vec3	FresnelSchlick(vec3 surfaceMetalColor, float cosTheta)
{
	return surfaceMetalColor + (vec3(1.0, 1.0, 1.0) - surfaceMetalColor) * pow(1.0 - cosTheta, 5.0);
}

void	ApplyLightBRDF(INOUT(SFragSurface) fSurf, vec3 surfToLight, vec3 surfToView, vec3 lightColor, vec3 surfaceDiffuseColor, float attenuation, float litMask FS_ARGS)
{
	vec3	halfVec = normalize(surfToLight + surfToView);
	
	float   NoL = dot(surfToLight, fSurf.m_Normal);

#if     defined(HAS_NormalWrap)
	float   normalWrapFactor = GET_CONSTANT(Material, NormalWrap_WrapFactor) * 0.5f;
	NoL = normalWrapFactor + (1.0f - normalWrapFactor) * NoL;
#endif

	NoL = max(0.0f, NoL) * attenuation;

	float	specIntensity = max(0.0f, dot(halfVec, fSurf.m_Normal));
	float	NoV = max(EPSILON, dot(surfToView, fSurf.m_Normal)); // Weird behavior when this is near 0

	vec3	surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceDiffuseColor, fSurf.m_Metalness);

	vec3	F = FresnelSchlick(surfaceMetalColor, max(0.0f, dot(halfVec, surfToView)));
	float	D = NormalDistFuncGGX(specIntensity, fSurf.m_Roughness);
	float	G = SchlickGGX(NoL, NoV, fSurf.m_Roughness);

	vec3	diffuseBRDF = mix(VEC3_ONE - F, VEC3_ZERO, fSurf.m_Metalness);
	vec3	specularBRDF = ((F * D * G) / max(EPSILON, 4.0 * NoL * NoV));

#	if defined(PK_LIGHTING_PASS)
	fSurf.m_LightAccu += surfaceDiffuseColor * (diffuseBRDF + specularBRDF) * lightColor * NoL;
#	else
	if (GET_CONSTANT(Material, Lit_Type) == 1)
	{
		fSurf.m_Diffuse.rgb += surfaceDiffuseColor * diffuseBRDF * lightColor * NoL;
		// We write the specular in the emissive output so that it doesn't get affected by the diffuse alpha:
		fSurf.m_Emissive += specularBRDF * lightColor * NoL * litMask;
	}
	else
	{
		fSurf.m_Diffuse.rgb += surfaceDiffuseColor * (diffuseBRDF + specularBRDF) * lightColor * NoL;
	}
#	endif
}

#	if defined(PK_FORWARD_COLOR_PASS)

vec3	FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

#		if !defined(HAS_FastLit) // function definition duplicate
vec3	toCubemapSpace(vec3 value FS_ARGS)
{
	vec3	valueLHZ = mul(GET_CONSTANT(SceneInfo, UserToLHZ), vec4(value, 0.0f)).xyz;
	vec3	valueRHY = vec3(valueLHZ.x, valueLHZ.z, valueLHZ.y);
	valueRHY.xz = mul(GET_CONSTANT(EnvironmentMapInfo, Rotation), valueRHY.xz);
	return valueRHY;
}
#		endif

void	ApplyAmbientBRDF(INOUT(SFragSurface) fSurf, vec3 surfToView, vec3 ambientColor, vec3 surfaceDiffuseColor, float litMask FS_ARGS)
{
	// Cubemaps are expected to be cosined filtered up to the 7th mipmap which should contain the fully diffuse 180 degrees cosine-filtered cubemap.
	const float	kCubemapMipRange = 6;

	vec3	surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceDiffuseColor, fSurf.m_Metalness);
	vec3	kS = FresnelSchlickRoughness(max(dot(fSurf.m_Normal, surfToView), 0.0), surfaceMetalColor, fSurf.m_Roughness);
	vec3	kD = (1.0f - kS) * (1.0f - fSurf.m_Metalness);

	// Ambient diffuse
	vec3	diffuseBRDF = kD * SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(fSurf.m_Normal FS_PARAMS), kCubemapMipRange).rgb;
	// Ambient specular
	vec3	sampleDir = toCubemapSpace(reflect(-surfToView, fSurf.m_Normal) FS_PARAMS);
	vec3	prefilteredColor = SAMPLELOD_CUBE(EnvironmentMapSampler, sampleDir, fSurf.m_Roughness * kCubemapMipRange).rgb;
	vec2	envBRDF = SAMPLE(BRDFLUTSampler, vec2(max(dot(fSurf.m_Normal, surfToView), 0.0), fSurf.m_Roughness)).rg;
	vec3	specularBRDF = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	if (GET_CONSTANT(Material, Lit_Type) == 1)
	{
		vec3	litDiffuse = surfaceDiffuseColor * diffuseBRDF * ambientColor;
		// Mix between the computed lit color and the original surface color
		// Completly overrides the previous computed lit color:
		fSurf.m_Diffuse.rgb = mix(surfaceDiffuseColor, litDiffuse, litMask);
		// We write the specular in the emissive output so that it doesn't get affected by the diffuse alpha:
		fSurf.m_Emissive += specularBRDF * ambientColor * litMask;
	}
	else
	{
		fSurf.m_Diffuse.rgb += surfaceDiffuseColor * (diffuseBRDF + specularBRDF) * ambientColor;
	}
}

#	endif // defined(PK_FORWARD_COLOR_PASS)

#endif // defined(PK_FORWARD_COLOR_PASS) || defined(PK_LIGHTING_PASS)

#if defined(HAS_Lit)

#	if defined(HAS_UVDistortions)
void	ApplyNormalMap(INOUT(SFragGeometry) fGeom, IN(SMaskDistortion)fMaskDistortion, bool isFrontFace FS_ARGS)
#	else
void	ApplyNormalMap(INOUT(SFragGeometry) fGeom, bool isFrontFace FS_ARGS)
#	endif
{
	float	crossSign = GET_CONSTANT(SceneInfo, Handedness);

#	if defined(HAS_UVDistortions)
	vec2 distortion = fMaskDistortion.m_UVDistortion1 * 0.1 + fMaskDistortion.m_UVDistortion2 * 0.01;
	vec3	normalTex = SampleDistortedTextureVec3(fGeom, distortion, SAMPLER_ARG(Lit_NormalMap) FS_PARAMS);
#	else
	vec3	normalTex = SampleTextureVec3(fGeom, SAMPLER_ARG(Lit_NormalMap) FS_PARAMS);
#	endif
	normalTex = vec3(2.0f, 2.0f, 2.0f) * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);
	// if UVs are rotated, inverse rotate the billboard space normal
	// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	normalTex.xy = mul(fGeom.m_TangentRotation, normalTex.xy);
#	endif // defined(HAS_TransformUVs)

	vec3	T = normalize(fGeom.m_Tangent.xyz);
	vec3	N = normalize(fGeom.m_Normal.xyz);
	vec3	B = CROSS(N, T) * crossSign * fGeom.m_Tangent.w;

	// Flip the normal for double sided particles:
	float	normalFlip = isFrontFace ? 1.0f : -1.0f;
	// Triangle renderers have an option to disable flipping the normal on backfaces:
#	if defined(HAS_TriangleCustomNormals)
	normalFlip = (GET_CONSTANT(Material, TriangleCustomNormals_DoubleSided) == 1) ? normalFlip : 1.0f;
#	endif

	mat3	TBN = BUILD_MAT3(T, B, N * normalFlip);

	fGeom.m_Normal = normalize(mul(TBN, normalTex.xyz));
}

#		if !defined(HAS_FastLit) // function definition duplicate
float 		remapValue(float value, float oldMin, float oldMax, float newMin, float newMax)
{
	return newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
}
#		endif

void	ApplyLighting(INOUT(SFragSurface) fSurf, IN(SFragGeometry) fGeom, vec3 surfacePosition, float roughness, float metalness FS_ARGS)
{
#	if defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
	vec2 roughMetal = SampleTextureVec2(fGeom, SAMPLER_ARG(Lit_RoughMetalMap) FS_PARAMS);
	fSurf.m_Roughness = roughness * roughMetal.x; 
	fSurf.m_Metalness = metalness * roughMetal.y;
#	endif
#	if defined(PK_FORWARD_COLOR_PASS)

	vec3	viewPosition = GET_MATRIX_W_AXIS(GET_CONSTANT(SceneInfo, UnpackNormalView)).xyz;
	vec3	surfToView = viewPosition - surfacePosition;
	vec3	surfToViewDir = normalize(surfToView);
	// Before lighting, we backup the surface diffuse color and reset the diffuse to 0 to accumulate the light influences:
	vec3	surfaceDiffuseColor = fSurf.m_Diffuse.rgb;
	fSurf.m_Diffuse.rgb = VEC3_ZERO;

	float	litMask = SampleTextureVec1(fGeom, SAMPLER_ARG(Lit_LitMaskMap) FS_PARAMS);
	vec3 	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);

	ApplyAmbientBRDF(fSurf, surfToViewDir, ambientColor, surfaceDiffuseColor, litMask FS_PARAMS);

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
		ApplyLightBRDF(fSurf, surfToLightDir, surfToViewDir, color, surfaceDiffuseColor, 1.0f, litMask FS_ARGS);
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

		ApplyLightBRDF(fSurf, surfToLightDir, surfToViewDir, color, surfaceDiffuseColor, lightAttenuation, litMask FS_ARGS);
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

		ApplyLightBRDF(fSurf, surfToLightDir, surfToViewDir, color, surfaceDiffuseColor, lightAttenuation, litMask FS_ARGS);
	}
#	endif // defined(PK_FORWARD_COLOR_PASS)
}

#elif defined(PK_LIGHTING_PASS)

// Julien's fall-off function:
// The constants A and B should be computed on CPU and passed as constants to the shader
//------------------------------
// k = 1.0 / steepness
// x = normalized distance
//------------------------------
//  float A = 1 / pow(k + 1, 2);
//  float B = 1 - A;
//  float C = 1 / pow(x * k + 1, 2);
//  return (C - A) / B;
//------------------------------

float	UnlinearizeFalloff(float x FS_ARGS)
{
	float	k = 1 / (GET_CONSTANT(Material, LightAttenuation_AttenuationSteepness) + 1.0e-3f);
	float	A = 1 / ((k + 1) * (k + 1));
	float	B = 1 - A;
	float	C = 1 / ((x * k + 1) * (x * k + 1));
	return (C - A) / B;
}

void	ApplyPointLight(INOUT(SFragSurface) fSurf, vec3 lightColor, vec3 lightPosition, float lightRange FS_ARGS)
{
	vec3	viewPosition = GET_MATRIX_W_AXIS(GET_CONSTANT(SceneInfo, UnpackNormalView)).xyz;
	vec3	surfToView = viewPosition - fSurf.m_WorldPosition;
	vec3	surfToLight = lightPosition - fSurf.m_WorldPosition;
	vec3	surfToLightDir = normalize(surfToLight);
	vec3	surfToViewDir = normalize(surfToView);
	float	sqRange = lightRange * lightRange;
	float	sqDistanceToLight = dot(surfToLight, surfToLight);

	if (sqDistanceToLight > sqRange || (fSurf.m_Roughness < 0.0f || fSurf.m_Metalness < 0.0f))
		discard; // Outside of the point light range

	float	distanceToLight = sqrt(sqDistanceToLight);

	// Compute the light attenuation:
	float	normalizedDistanceToLight = min(1.0f, distanceToLight / lightRange);
	float	lightAttenuation = UnlinearizeFalloff(normalizedDistanceToLight FS_PARAMS);

	ApplyLightBRDF(fSurf, surfToLightDir, surfToViewDir, lightColor, fSurf.m_Diffuse, lightAttenuation, 0.0f FS_PARAMS);
}

#endif // defined(HAS_Lit)
