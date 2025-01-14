
vec2 PackNormalSpheremap(vec3 normal FS_ARGS)
{
	normal = normalize(normal);
	normal = mul(GET_CONSTANT(SceneInfo, PackNormalView), vec4(normal, 0.0f)).xyz;
	float f = sqrt(8.0f * normal.z + 8.0f);
	return normal.xy / f + 0.5f;
}

#if defined(HAS_TransformUVs)
vec2 transformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}
#endif

void     FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	float   roughness = -1.0f;
	float   metalness = -1.0f;
    vec3    normal = vec3(1,0,0);
    vec4    color = vec4(0,0,0,0);
    vec4    emissive = vec4(0,0,0,0);

#if     defined(FINPUT_fragDiffuse_Color)
    color = fInput.fragDiffuse_Color;
#else
    color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
#endif

#if     defined(HAS_Diffuse) || defined(HAS_Lit) || defined(HAS_LegacyLitOpaque) || defined(HAS_Emissive) // has texture sampling

	vec2    fragUV0 = fInput.fragUV0;

#if		defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	vec2    fragUV1 = fInput.fragUV1;

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
	// For clean derivatives:
	vec2	_uv = fragUV0 * rect0.xy + rect0.zw;
	vec2	dUVdx = dFdx(_uv);
	vec2	dUVdy = dFdy(_uv);
	fragUV0 = fract(fragUV0) * rect0.xy + rect0.zw; // undo normalize
	bool	RGBOnly = GET_CONSTANT(Material, TransformUVs_RGBOnly) != 0;
#endif

#if	defined(HAS_Diffuse)
#	if defined(HAS_TransformUVs)
	vec4	textureColor = SAMPLEGRAD(Diffuse_DiffuseMap, fragUV0, dUVdx, dUVdy);
#	else
	vec4	textureColor = SAMPLE(Diffuse_DiffuseMap, fragUV0);
#	endif
	
#	if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor.a =  SAMPLE(Diffuse_DiffuseMap, oldFragUV0).a;
		}
#	endif

#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
#		if defined(HAS_TransformUVs)
		vec4    textureColor2 = SAMPLEGRAD(Diffuse_DiffuseMap, fragUV1, dUVdx, dUVdy);
#		else
		vec4    textureColor2 = SAMPLE(Diffuse_DiffuseMap, fragUV1);
#		endif
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

#if defined(HAS_Opaque)
    if (GET_CONSTANT(Material, Opaque_Type) == 1 &&
		color.a < GET_CONSTANT(Material, Opaque_MaskThreshold))
        discard;
#endif

#if 	defined(ParticlePass_OpaqueShadow)
	float 	shadowDepth = fInput.fragViewProjPosition.z / fInput.fragViewProjPosition.w;
	fOutput.Output0 = vec2(shadowDepth, shadowDepth * shadowDepth);

#else // !ParticlePass_OpaqueShadow

#if     defined(HAS_Lit)

#if defined(HAS_TransformUVs)
	vec3 normalTex = SAMPLEGRAD(Lit_NormalMap, fragUV0, dUVdx, dUVdy).xyz;	
#else
	vec3 normalTex = SAMPLE(Lit_NormalMap, fragUV0).xyz;
#endif

#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec3 normalTex2 = SAMPLEGRAD(Lit_NormalMap, fragUV1, dUVdx, dUVdy).xyz;	
#else
		vec3 normalTex2 = SAMPLE(Lit_NormalMap, fragUV1).xyz;
#endif
		normalTex = mix(normalTex, normalTex2, blendMix);
	}
#endif

	normalTex = 2.0f * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);
	
// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	float sinMR = -sinR; // sin(-x) = -sin(x)
	float cosMR = cosR; // cos(-x) = cos(x)
	mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
	normalTex.xy = mul(UVInverseRotation, normalTex.xy);
#	endif

	vec3    T = normalize(fInput.fragTangent.xyz);
	vec3    N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3    TBN = BUILD_MAT3(T, B, N);

	normal = normalize(mul(TBN, normalTex));

	vec2    roughMetal = SAMPLE(Lit_RoughMetalMap, fragUV0).xy;
#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec2    roughMetal2 = SAMPLE(Lit_RoughMetalMap, fragUV1).xy;
		roughMetal = mix(roughMetal, roughMetal2, blendMix);
	}
#endif

	roughness = GET_CONSTANT(Material, Lit_Roughness);
	roughness *= roughMetal.x;
	metalness = GET_CONSTANT(Material, Lit_Metalness);
	metalness *= roughMetal.y;

#elif   defined(HAS_LegacyLitOpaque)

#if defined(HAS_TransformUVs)
	vec3 normalTex =  SAMPLEGRAD(LegacyLitOpaque_NormalMap, fragUV0, dUVdx, dUVdy).xyz;
#else
	vec3 normalTex =  SAMPLE(LegacyLitOpaque_NormalMap, fragUV0).xyz;
#endif
#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec3 normalTex2 =  SAMPLEGRAD(LegacyLitOpaque_NormalMap, fragUV1, dUVdx, dUVdy).xyz;
#else
		vec3 normalTex2 =  SAMPLE(LegacyLitOpaque_NormalMap, fragUV1).xyz;
#endif
		normalTex = mix(normalTex, normalTex2, blendMix);
	}
#endif

	normalTex = 2.0f * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);
	
// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if defined(HAS_TransformUVs)
	float sinMR = -sinR; // sin(-x) = -sin(x)
	float cosMR = cosR; // cos(-x) = cos(x)
	mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
	normalTex.xy = mul(UVInverseRotation, normalTex.xy);
#	endif	
	
	vec3    T = normalize(fInput.fragTangent.xyz);
	vec3    N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3    TBN = BUILD_MAT3(T, B, N);
	normal = normalize(mul(TBN, normalTex));

	// In the old lighting feature, we stored the specular and glossiness in the specular map.
	// To mimic something similar, we just set the roughness to (1.0 - specularValue) * 0.7 + 0.3 to avoid very sharp specular which did not exist before
	// The metalness is always 0.
	roughness = (1.0 - SAMPLE(LegacyLitOpaque_SpecularMap, fragUV0).x) * 0.7 + 0.3;
#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		float roughness2 = (1.0 - SAMPLE(LegacyLitOpaque_SpecularMap, fragUV1).x) * 0.7 + 0.3;
		roughness = mix(roughness, roughness2, blendMix);
	}
#endif
	metalness = 0.0f;

#else
	emissive = color;
	color = vec4(0.0,0.0,0.0,0.0);

#endif

#if defined(HAS_Emissive)
#if defined(HAS_TransformUVs)
	vec3	emissiveColor1 = SAMPLEGRAD(Emissive_EmissiveMap, fragUV0, dUVdx, dUVdy).rgb;
#else
	vec3	emissiveColor1 = SAMPLE(Emissive_EmissiveMap, fragUV0).rgb;
#endif

#if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
#if defined(HAS_TransformUVs)
		vec3 emissiveColor2 = SAMPLEGRAD(Emissive_EmissiveMap, fragUV1, dUVdx, dUVdy).rgb;
#else
		vec3 emissiveColor2 = SAMPLE(Emissive_EmissiveMap, fragUV1).rgb;
#endif
		emissiveColor1 = mix(emissiveColor1, emissiveColor2, blendMix);
	}
#endif

#if defined(HAS_EmissiveRamp)
	emissiveColor1 = SAMPLE(EmissiveRamp_RampMap, vec2(emissiveColor1.x,0.0)).rgb;
#endif

	emissiveColor1 *= fInput.fragEmissive_EmissiveColor;

	emissive.rgb += emissiveColor1;
#endif

	vec4    normalSpec;
	normalSpec = vec4(PackNormalSpheremap(normal FS_PARAMS), roughness, metalness);

    fOutput.Output0 = color;
    fOutput.Output1 = fInput.fragViewProjPosition.z / fInput.fragViewProjPosition.w;
    fOutput.Output2 = emissive;
    fOutput.Output3 = normalSpec;
#endif // !ParticlePass_OpaqueShadow
}
