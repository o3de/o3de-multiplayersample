
#if     defined(HAS_SoftParticles)
float   clipToLinearDepth(float depth FS_ARGS)
{
	float zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	return (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
}
#endif

#if     defined(HAS_Distortion)
float   clipDepthToDistortionFade(float depth FS_ARGS)
{
	float zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	float zLinear = (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
	return min(zLinear / zNear - 1.f, sqrt(10.f / zLinear) ); // looks like the v1 (10.f = sqrt(zFar * zNear))
}
#endif

#if defined(HAS_Lit) && (defined(ParticlePass_TransparentPostDisto) || defined(ParticlePass_Transparent))

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

vec3	computeBRDF(vec3 surfToLight, vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec4 diffuseColor, float litMask FS_ARGS)
{
	vec3    halfVec = normalize(surfToLight + surfToView);

	float   NoL = max(0.0f, dot(surfToLight, surfaceNormal));

#if     defined(HAS_NormalWrap)
	float   normalWrapFactor = GET_CONSTANT(Material, NormalWrap_WrapFactor);
	NoL = normalWrapFactor + (1.0f - normalWrapFactor) * NoL;
#endif

	float   specIntensity = max(0.0f, dot(halfVec, surfaceNormal));
	float   NoV = max(EPSILON, dot(surfToView, surfaceNormal)); // Weird behavior when this is near 0

	vec3    surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, diffuseColor.rgb, metalness);

	vec3    F  = fresnelSchlick(surfaceMetalColor, max(0.0f, dot(halfVec, surfToView)));
	float   D = normalDistFuncGGX(specIntensity, roughness);
	float   G = schlickGGX(NoL, NoV, roughness);

	float specularMultiplier = GET_CONSTANT(Material, Lit_Type) == 1 ? litMask : diffuseColor.a;
	vec3 diffuseBRDF = mix(vec3(1.0, 1.0, 1.0) - F, vec3(0.0, 0.0, 0.0), metalness) * diffuseColor.rgb * diffuseColor.a;
	vec3 specularBRDF = ((F * D * G) / max(EPSILON, 4.0 * NoL * NoV)) * specularMultiplier;

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
	valueRHY.xz = mul(GET_CONSTANT(EnvironmentMapInfo, Rotation), valueRHY.xz);
	return valueRHY;
}

vec3	computeAmbientBRDF(vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec4 diffuseColor, float litMask FS_ARGS)
{
	// Cubemaps are expected to be cosined filtered up to the 7th mipmap which should contain the fully diffuse 180 degrees cosine-filtered cubemap.
	const float	kCubemapMipRange = 6;

	vec3    surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, diffuseColor.rgb, metalness);
	vec3	kS = fresnelSchlickRoughness(max(dot(surfaceNormal, surfToView), 0.0), surfaceMetalColor, roughness);
	vec3	kD = 1.0 - kS;
	kD *= 1.0 - metalness;

	// Ambient diffuse
	vec3	irradiance = SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(surfaceNormal FS_PARAMS), kCubemapMipRange).rgb;
	vec3	diffuse = irradiance * diffuseColor.rgb * diffuseColor.a;

	// Ambient specular
	vec3	prefilteredColor = SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(reflect(-surfToView, surfaceNormal) FS_PARAMS), roughness * kCubemapMipRange).rgb;
	vec2	envBRDF  = SAMPLE(BRDFLUTSampler, vec2(max(dot(surfaceNormal, surfToView), 0.0), roughness)).rg;
	float 	specularMultiplier = GET_CONSTANT(Material, Lit_Type) == 1 ? litMask : diffuseColor.a;
	vec3	specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y) * specularMultiplier;

	vec3 ambient = kD * diffuse + specular;
	return ambient;
}

#endif

#if defined(HAS_TransformUVs)
vec2 transformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}
#endif

vec2    applyCorrectDeformation(IN(SFragmentInput) fInput, IN(vec2) fragUVin FS_ARGS)
{
#if     defined(HAS_CorrectDeformation)
	vec2 fragUVout;
	if (fragUVin.x + fragUVin.y < 1)
		fragUVout = fragUVin.xy / fInput.fragUVFactors.xy;
	else
		fragUVout = 1.0 - ((1.0 - fragUVin.xy) / fInput.fragUVFactors.zw);
#if     defined(HAS_TextureUVs)
	if (GET_CONSTANT(Material, TextureUVs_RotateTexture) != 0)
		fragUVout = fragUVout.yx;
#endif
	return fragUVout * fInput.fragUVScaleAndOffset.xy + fInput.fragUVScaleAndOffset.zw;
#else
	return fragUVin;
#endif
}

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
//----------------------------------------------------------------------------
// Gather texture coordinates
//----------------------------------------------------------------------------
#if 	defined(FINPUT_fragUV0)
	vec2    fragUV0 = applyCorrectDeformation(fInput, fInput.fragUV0 FS_PARAMS);
#	if		defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	vec2	fragUV1 = applyCorrectDeformation(fInput, fInput.fragUV1 FS_PARAMS);
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
#	endif // defined(HAS_Atlas)
#endif // defined(FINPUT_fragUV0)

//----------------------------------------------------------------------------
// Transform UVs
//----------------------------------------------------------------------------
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
#	endif // defined(HAS_Atlas)
	vec2	oldFragUV0 = fragUV0;	
	fragUV0 = ((fragUV0 - rect0.zw) / rect0.xy); // normalize (if atlas)
	fragUV0 = transformUV(fragUV0, UVScale, UVRotation, UVOffset); // scale then rotate then translate UV
	fragUV0 = fract(fragUV0) * rect0.xy + rect0.zw; // undo normalize
	bool	RGBOnly = GET_CONSTANT(Material, TransformUVs_RGBOnly) != 0;
#endif // defined(HAS_TransformUVs)

//----------------------------------------------------------------------------
// render pass distortion
//----------------------------------------------------------------------------
#if 	defined(ParticlePass_Distortion)

	vec4    distoColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

#if     defined(FINPUT_fragDistortion_Color)
	distoColor = fInput.fragDistortion_Color;
#endif

#if     defined(HAS_SoftParticles) || defined(HAS_Distortion)
	vec4    projPos = fInput.fragViewProjPosition;
	float   rcpw = 1.0 / projPos.w;
	float   fragDepth_cs = projPos.z * rcpw;
	//  if (IsDepthReversed())
	//      fragDepth_cs = 1.0 - fragDepth_cs;
#endif

	vec4    textureColor;
	
	distoColor *= clipDepthToDistortionFade(fragDepth_cs  FS_PARAMS);

#   if      defined(HAS_Atlas)
	
	if (blendingType == 2)
	{
		vec4	textureColor1 = SAMPLE(Distortion_DistortionMap, fragUV0);
		vec4	textureColor2 = SAMPLE(Distortion_DistortionMap, fragUV1);
		textureColor = mix(textureColor1, textureColor2, blendMix);
	}
	else
	{
		textureColor = SAMPLE(Distortion_DistortionMap, fragUV0);
		if (blendingType == 1)
		{
			// Linear
			vec4    textureColor2 = SAMPLE(Distortion_DistortionMap, fragUV1);
			textureColor = mix(textureColor, textureColor2, blendMix);
		}
	}
#	else
	// No atlas
	textureColor = SAMPLE(Distortion_DistortionMap, fInput.fragUV0);
#	endif // defined(HAS_Atlas)

	distoColor *= textureColor * vec4(2., 2., 1., 0.) - vec4(1.00392, 1.00392, 0., -1.); // 128 is considered as the mid-value (in range 0-255)

#if     defined(HAS_SoftParticles)
	vec2    screenUV = projPos.xy * rcpw * vec2(0.5, 0.5) + 0.5;
	float   sceneDepth_cs = SAMPLE(DepthSampler, screenUV).x;
	float   sceneDepth = clipToLinearDepth(sceneDepth_cs FS_PARAMS);
	float   fragDepth = clipToLinearDepth(fragDepth_cs FS_PARAMS);
	float   invSoftnessDistance = 1.0f / GET_CONSTANT(Material, SoftParticles_SoftnessDistance);
	float   depthfade = clamp((sceneDepth - fragDepth) * invSoftnessDistance, 0.f, 1.f);
	distoColor *= depthfade;
#endif

	fOutput.Output0 = distoColor;

//----------------------------------------------------------------------------
// render pass transparent
//----------------------------------------------------------------------------
#elif 	defined(ParticlePass_TransparentPostDisto) || defined(ParticlePass_Transparent)

	vec4    diffuseColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec3    emissiveColor = vec3(0.0f, 0.0f, 0.0f);

//----------------------------------------------------------------------------
// DIFFUSE
//----------------------------------------------------------------------------
#if	defined(HAS_Diffuse)
	diffuseColor = fInput.fragDiffuse_Color;

	vec4    textureColor = SAMPLE(Diffuse_DiffuseMap, fragUV0);
#	if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor.a =  SAMPLE(Diffuse_DiffuseMap, oldFragUV0).a;
		}
#	endif // defined(HAS_TransformUVs)

#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    textureColor2 = SAMPLE(Diffuse_DiffuseMap, fragUV1);
#		if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor2.a =  SAMPLE(Diffuse_DiffuseMap, oldFragUV1).a;
		}
#		endif // defined(HAS_TransformUVs)
		textureColor = mix(textureColor, textureColor2, blendMix);
	}
#	endif // defined(HAS_Atlas)

	diffuseColor *= textureColor;

#	if defined(HAS_DiffuseRamp)
	diffuseColor.rgb =  SAMPLE(DiffuseRamp_RampMap, vec2(diffuseColor.x, 0.0)).rgb;
#	endif // defined(HAS_DiffuseRamp)

#if	defined(HAS_AlphaRemap)
	vec2    alphaTexCoord = vec2(diffuseColor.a, fInput.fragAlphaRemap_Cursor);
	diffuseColor.a = SAMPLE(AlphaRemap_AlphaMap, alphaTexCoord).r;
#endif // defined(HAS_AlphaRemap)
#endif // defined(HAS_Diffuse)

//----------------------------------------------------------------------------
// LIT
//----------------------------------------------------------------------------
#if defined(HAS_Lit)
	vec3 surfacePosition = fInput.fragWorldPosition;

	float litMask1 = SAMPLE(Lit_LitMaskMap, fragUV0).r;
	vec3 normalTex1 = SAMPLE(Lit_NormalMap, fragUV0).xyz;
#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec3 normalTex2 =  SAMPLE(Lit_NormalMap, fragUV1).xyz;
		float litMask2 = SAMPLE(Lit_LitMaskMap, fragUV1).r;

		normalTex1 = mix(normalTex1, normalTex2, blendMix);
		litMask1 = mix(litMask1, litMask2, blendMix);
	}
#endif // defined(HAS_Atlas)
	normalTex1 = 2.0f * normalTex1.xyz - vec3(1.0f, 1.0f, 1.0f);
	diffuseColor.a *= litMask1;

// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	float sinMR = -sinR; // sin(-x) = -sin(x)
	float cosMR = cosR; // cos(-x) = cos(x)
	mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
	normalTex1.xy = mul(UVInverseRotation, normalTex1.xy);
#	endif // defined(HAS_TransformUVs)

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
										diffuseColor,
										litMask1
										FS_PARAMS);

	vec3	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);
	vec3	lightAmbient = computeAmbientBRDF(	surfToViewDir,
												surfaceNormal,
												surfaceRoughness,
												surfaceMetalness,
												diffuseColor,
												litMask1
												FS_PARAMS);

	diffuseColor.rgb = lightDiffuse * lightColor + lightAmbient * ambientColor;
#else // !defined(HAS_Lit)
	diffuseColor.rgb *= diffuseColor.a; // If we are unlit, we fade the diffuse color by it's alpha value
#endif

//----------------------------------------------------------------------------
// EMISSIVE
//----------------------------------------------------------------------------
#if	defined(HAS_Emissive)

#if     defined(FINPUT_fragEmissive_EmissiveColor)
	emissiveColor = fInput.fragEmissive_EmissiveColor;
#endif

	vec3	emissiveColor1 = SAMPLE(Emissive_EmissiveMap, fragUV0).rgb;

#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec3 emissiveColor2 = SAMPLE(Emissive_EmissiveMap, fragUV1).rgb;
		emissiveColor1 = mix(emissiveColor1, emissiveColor2, blendMix);
	}
#endif // defined(HAS_Atlas)

	emissiveColor *= emissiveColor1;

#if	defined(HAS_EmissiveRamp)
	emissiveColor = SAMPLE(EmissiveRamp_RampMap, vec2(emissiveColor.x,0.0)).rgb;
#endif // defined(HAS_EmissiveRamp)

#endif // defined(HAS_Emissive)

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
	emissiveColor *= depthfade;
	diffuseColor.a *= depthfade;
#endif // defined(HAS_SoftParticles)

	fOutput.Output0 = vec4(emissiveColor + diffuseColor.rgb, diffuseColor.a);

// !defined(ParticlePass_TransparentPostDisto)
#elif defined(ParticlePass_Tint)

	vec4	tintColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

//----------------------------------------------------------------------------
// Tint
//----------------------------------------------------------------------------
#if     defined(FINPUT_fragTint_Color)
	tintColor = fInput.fragTint_Color;
#endif

#if	defined(HAS_Tint)
	vec4    textureColor = SAMPLE(Tint_TintMap, fragUV0);
#	if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor.a =  SAMPLE(Tint_TintMap, oldFragUV0).a;
		}
#	endif // defined(HAS_TransformUVs)

#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    textureColor2 = SAMPLE(Tint_TintMap, fragUV1);
#		if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor2.a =  SAMPLE(Tint_TintMap, oldFragUV1).a;
		}
#		endif // defined(HAS_TransformUVs)
		textureColor = mix(textureColor, textureColor2, blendMix);
	}
#	endif // defined(HAS_Atlas)

	tintColor *= textureColor;
	tintColor.rgb = mix(vec3(1.0f, 1.0f, 1.0f), tintColor.rgb, tintColor.a);

#endif // defined(HAS_Tint)

	fOutput.Output0 = tintColor;

#endif

}
