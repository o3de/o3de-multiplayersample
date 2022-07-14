
#if defined(HAS_SoftParticles)
float   clipToLinearDepth(float depth FS_ARGS)
{
	float zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	return (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
}
#endif

#if defined(HAS_Lit)

//------------------------------
// BRDF Computation
//------------------------------
#   define PI                       3.141592
#   define  METAL_FRESNEL_FACTOR    vec3(0.04, 0.04, 0.04)
#   define EPSILON                  1e-5

float normalDistFuncGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float schlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // UE4 remapping

	float t1 = cosLi / (cosLi * (1.0 - k) + k);
	float t2 = cosLo / (cosLo * (1.0 - k) + k);

	return t1 * t2;
}
 
vec3 fresnelSchlick(vec3 surfaceMetalColor, float cosTheta)
{
	return surfaceMetalColor + (vec3(1.0, 1.0, 1.0) - surfaceMetalColor) * pow(1.0 - cosTheta, 5.0);
}

vec3	computeBRDF(vec3 surfToLight, vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec3 surfaceColor FS_ARGS)
{
	vec3    halfVec = normalize(surfToLight + surfToView);

	float   NoL = max(0.0f, dot(surfToLight, surfaceNormal));

#if     defined(HAS_NormalWrap)
	float   normalWrapFactor = GET_CONSTANT(Material, NormalWrap_WrapFactor);
	NoL = normalWrapFactor + (1.0f - normalWrapFactor) * NoL;
#endif

	float   specIntensity = max(0.0f, dot(halfVec, surfaceNormal));
	float   NoV = max(EPSILON, dot(surfToView, surfaceNormal)); // Weird behavior when this is near 0

	vec3    surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceColor, metalness);

	vec3    F  = fresnelSchlick(surfaceMetalColor, max(0.0f, dot(halfVec, surfToView)));
	float   D = normalDistFuncGGX(specIntensity, roughness);
	float   G = schlickGGX(NoL, NoV, roughness);

	vec3 diffuseBRDF = mix(vec3(1.0, 1.0, 1.0) - F, vec3(0.0, 0.0, 0.0), metalness) * surfaceColor;
	vec3 specularBRDF = (F * D * G) / max(EPSILON, 4.0 * NoL * NoV);

	return (diffuseBRDF + specularBRDF) * NoL;
}

vec3	fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

vec3	toCubemapSpace(vec3 value FS_ARGS)
{
	vec3	valueLHZ = mul(GET_CONSTANT(SceneInfo, UserToLHZ), vec4(value, 0.0f)).xyz;
	vec3	valueRHY = vec3(valueLHZ.x, valueLHZ.z, valueLHZ.y);
	return valueRHY;
}

vec3	computeAmbientBRDF(vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec3 surfaceColor FS_ARGS)
{
	// Cubemaps are expected to be cosined filtered up to the 7th mipmap which should contain the fully diffuse 180 degrees cosine-filtered cubemap.
	const float	kCubemapMipRange = 6;

	vec3	surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceColor, metalness);
	vec3	kS = fresnelSchlickRoughness(max(dot(surfaceNormal, surfToView), 0.0), surfaceMetalColor, roughness);
	vec3	kD = 1.0f - kS;
	kD *= 1.0f - metalness;

	// Ambient diffuse
	vec3	irradiance = SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(surfaceNormal FS_PARAMS), kCubemapMipRange).rgb;
	vec3	diffuse = irradiance * surfaceColor;

	// Ambient specular
	vec3	sampleDir = toCubemapSpace(reflect(-surfToView, surfaceNormal) FS_PARAMS);
	vec3	prefilteredColor = SAMPLELOD_CUBE(EnvironmentMapSampler, sampleDir, roughness * kCubemapMipRange).rgb;
	vec2	envBRDF  = SAMPLE(BRDFLUTSampler, vec2(max(dot(surfaceNormal, surfToView), 0.0), roughness)).rg;
	vec3	specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	vec3	ambient = kD * diffuse + specular;
	
	return ambient;
}

#endif

#if defined(HAS_TransformUVs)
vec2 transformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}
#endif

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec4    color;

#if     defined(FINPUT_fragDiffuse_Color)
	color = fInput.fragDiffuse_Color;
#else
	color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
#endif

#if     defined(HAS_Diffuse) || defined(HAS_Lit) || defined(HAS_LegacyLit) || defined(HAS_Emissive) // has texture sampling

	vec2    fragUV0 = fInput.fragUV0;

#if		defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	vec2	fragUV1 = fInput.fragUV1;

	float	blendMix = 0.f;

	if (blendingType == 2)
	{
		// Motion vectors
		vec2	scale = GET_CONSTANT(Material, Atlas_DistortionStrength);
		vec2	curVectors = ((SAMPLE(Atlas_MotionVectorsMap, fragUV0).rg * 2.0f) - 1.0f) * scale;
		vec2	nextVectors = ((SAMPLE(Atlas_MotionVectorsMap, fragUV1).rg * 2.0f) - 1.0f) * scale;
		float	cursor = fract(fInput.fragAtlas_TextureID);

		curVectors *= cursor;
		nextVectors *= (1.0f - cursor);

		fragUV0 = fragUV0 - curVectors;
		fragUV1 = fragUV1 + nextVectors;
		blendMix = cursor;
	}
	else if (blendingType == 1)
	{
		// Linear
		blendMix = fract(fInput.fragAtlas_TextureID);
	}
#endif // defined(HAS_Atlas)

#endif // has texture sampling

#if	defined(HAS_TransformUVs)
	float	sinR = sin(fInput.fragTransformUVs_UVRotate);
	float	cosR = cos(fInput.fragTransformUVs_UVRotate);
	mat2	UVRotation = mat2(cosR, sinR, -sinR, cosR);
	vec2	UVScale = fInput.fragTransformUVs_UVScale;
	vec2	UVOffset = fInput.fragTransformUVs_UVOffset;
	vec4	rect0 = vec4(1.0f, 1.0f, 0.0f, 0.0f);
#	if	defined(HAS_Atlas)
	rect0 = LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID)) * 4 + 1));
	vec4	rect1 = LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID) + 1) * 4 + 1));

	vec2	oldFragUV1 = fragUV1;
	fragUV1 = ((fragUV1 - rect1.zw) / rect1.xy); // normalize (if atlas)
	fragUV1 = transformUV(fragUV1, UVScale, UVRotation, UVOffset); // scale then rotate then translate UV
	fragUV1 = fract(fragUV1) * rect1.xy + rect1.zw; // undo normalize
#	endif
	vec2	oldFragUV0 = fragUV0;	
	fragUV0 = ((fragUV0 - rect0.zw) / rect0.xy); // normalize (if atlas)
	fragUV0 = transformUV(fragUV0, UVScale, UVRotation, UVOffset); // scale then rotate then translate UV
	fragUV0 = fract(fragUV0) * rect0.xy + rect0.zw; // undo normalize
	bool	RGBOnly = GET_CONSTANT(Material, TransformUVs_RGBOnly) != 0;
#endif

#if	defined(HAS_Diffuse)
	vec4    textureColor = SAMPLE(Diffuse_DiffuseMap, fragUV0);
	
#	if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor.a =  SAMPLE(Diffuse_DiffuseMap, oldFragUV0).a;
		}
#	endif

#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    textureColor2 = SAMPLE(Diffuse_DiffuseMap, fragUV1);
#		if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor2.a =  SAMPLE(Diffuse_DiffuseMap, oldFragUV1).a;
		}
#		endif
		textureColor = mix(textureColor, textureColor2, blendMix);
	}
#	endif

#if defined(HAS_DiffuseRamp)
	textureColor.rgb = SAMPLE(DiffuseRamp_RampMap, vec2(textureColor.x,0.0)).rgb;
#endif

	color *= textureColor;

#if	defined(HAS_AlphaRemap)
	vec2    alphaTexCoord = vec2(color.a, fInput.fragAlphaRemap_Cursor);
	color.a = SAMPLE(AlphaRemap_AlphaMap, alphaTexCoord).r;
#endif 

#endif // defined HAS_Diffuse

#if	defined(HAS_SoftParticles)
	vec4    projPos = fInput.fragViewProjPosition;
	float   rcpw = 1.0 / projPos.w;
	float   fragDepth_cs = projPos.z * rcpw;
	//  if (IsDepthReversed())
	//      fragDepth_cs = 1.0 - fragDepth_cs;
	vec2    screenUV = projPos.xy * rcpw * vec2(0.5, 0.5) + 0.5;
	float   sceneDepth_cs = SAMPLE(DepthSampler, screenUV).x;
	float   sceneDepth = clipToLinearDepth(sceneDepth_cs FS_PARAMS);
	float   fragDepth = clipToLinearDepth(fragDepth_cs FS_PARAMS);
	float   invSoftnessDistance = 1.0f / GET_CONSTANT(Material, SoftParticles_SoftnessDistance);
	float   depthfade = clamp((sceneDepth - fragDepth) * invSoftnessDistance, 0.f, 1.f);
	color *= depthfade;
#endif

#if defined(HAS_Lit)
	vec3 surfacePosition = fInput.fragWorldPosition;

	vec3 normalTex1 = SAMPLE(Lit_NormalMap, fragUV0).xyz;
#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec3 normalTex2 =  SAMPLE(Lit_NormalMap, fragUV1).xyz;
		normalTex1 = mix(normalTex1, normalTex2, blendMix);
	}
#endif
	normalTex1 = 2.0f * normalTex1.xyz - vec3(1.0f, 1.0f, 1.0f);

// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	float sinMR = -sinR; // sin(-x) = -sin(x)
	float cosMR = cosR; // cos(-x) = cos(x)
	mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
	normalTex1.xy = mul(UVInverseRotation, normalTex1.xy);
#	endif

	vec3	T = normalize(fInput.fragTangent.xyz);
	vec3	N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3	TBN = BUILD_MAT3(T, B, N);

	vec3	surfaceNormal = normalize(mul(TBN, normalTex1.xyz));

	float 	surfaceRoughness = GET_CONSTANT(Material, Lit_Roughness);
	float	surfaceMetalness = GET_CONSTANT(Material, Lit_Metalness);

	bool 	hasDirLight = GET_CONSTANT(LightInfo, DirectionalLightsCount) >= 1;
	vec3 	lightDirection = hasDirLight ? LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(0)) : vec3(0, 1, 0);
	vec3 	lightColor = hasDirLight ? LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(8)) : VEC3_ZERO;

	vec3	viewPosition = GET_MATRIX_W_AXIS(GET_CONSTANT(SceneInfo, UnpackNormalView)).xyz;
	vec3	surfToView = viewPosition - surfacePosition;
	vec3	surfToLight = -lightDirection;
	vec3	surfToLightDir = normalize(surfToLight);
	vec3	surfToViewDir = normalize(surfToView);

	vec3	lightDiffuse = computeBRDF(	surfToLightDir,
										surfToViewDir,
										surfaceNormal,
										surfaceRoughness,
										surfaceMetalness,
										color.rgb
										FS_PARAMS);

	vec3	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);
	vec3	lightAmbient = computeAmbientBRDF(	surfToViewDir,
												surfaceNormal,
												surfaceRoughness,
												surfaceMetalness,
												color.rgb
												FS_PARAMS);

	color.rgb = lightDiffuse * lightColor + lightAmbient * ambientColor;

#elif defined(HAS_LegacyLit)

	vec3 normalTex1 =  SAMPLE(LegacyLit_NormalMap, fragUV0).xyz;
#if 	defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec3 normalTex2 =  SAMPLE(LegacyLit_NormalMap, fragUV1).xyz;
		normalTex1 = mix(normalTex1, normalTex2, blendMix);
	}
#endif

	normalTex1 = 2.0f * normalTex1.xyz - vec3(1.0f, 1.0f, 1.0f);
	
// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	float sinMR = -sinR; // sin(-x) = -sin(x)
	float cosMR = cosR; // cos(-x) = cos(x)
	mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
	normalTex1.xy = mul(UVInverseRotation, normalTex1.xy);
#	endif

	vec3	T = normalize(fInput.fragTangent.xyz);
	vec3	N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3	TBN = BUILD_MAT3(T, B, N);

	vec3 surfaceNormal = normalize(mul(TBN, normalTex1.xyz));

	bool 	hasDirLight = GET_CONSTANT(LightInfo, DirectionalLightsCount) >= 1;
	vec3 	lightDirection = hasDirLight ? LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(0)) : vec3(0, 1, 0);
	vec3 	lightColor = hasDirLight ? LOADF3(GET_RAW_BUFFER(DirectionalLightsInfo), RAW_BUFFER_INDEX(8)) : VEC3_ZERO;

	// Compute the light incidence
	vec3	posToLightDirection = -normalize(lightDirection); // for the directional lights, instead of the position, we get the orientation
	float	incidenceRemap = GET_CONSTANT(Material, LegacyLit_NormalWrapFactor) * 0.5f;
	float	lightIncidence = max(0.f, incidenceRemap + (1.f - incidenceRemap) * dot(posToLightDirection, surfaceNormal));
	lightIncidence = pow(lightIncidence, GET_CONSTANT(Material, LegacyLit_LightExponent));
	vec3	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);
	//  Compute the ambient light
	vec3	lightAmbient = 	ambientColor + GET_CONSTANT(Material, LegacyLit_AmbientLight);
	// Final light factor
	color.rgb *= (lightColor * lightIncidence + lightAmbient) * GET_CONSTANT(Material, LegacyLit_LightScale);
#endif

#if	defined(HAS_Emissive)
	vec3	emissiveColor1 = SAMPLE(Emissive_EmissiveMap, fragUV0).rgb;

#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec3 emissiveColor2 = SAMPLE(Emissive_EmissiveMap, fragUV1).rgb;
		emissiveColor1 = mix(emissiveColor1, emissiveColor2, blendMix);
	}
#endif

#if	defined(HAS_EmissiveRamp)
	emissiveColor1 = SAMPLE(EmissiveRamp_RampMap, vec2(emissiveColor1.x,0.0)).rgb;
#endif

	emissiveColor1 *= fInput.fragEmissive_EmissiveColor;

	color.rgb += emissiveColor1.rgb;
#endif	

	fOutput.Output0 = color;
}
