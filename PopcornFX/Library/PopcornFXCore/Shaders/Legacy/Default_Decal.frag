
vec3    DepthToWorldPos(float pDepthValue, vec2 pUv, mat4 pInvViewProj)
{
	vec2    clipSpace = pUv * 2.0 - 1.0;
	vec4    projPos = vec4(clipSpace, pDepthValue, 1.0);
	projPos = mul(pInvViewProj, projPos);
	return projPos.xyz / projPos.w;
}

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec2    screenTexCoord = fInput.fragViewProjPosition.xy / fInput.fragViewProjPosition.w * 0.5 + 0.5;
	float   depthValue = SAMPLE(DepthSampler, screenTexCoord).r;
	mat4    invViewProj = GET_CONSTANT(SceneInfo, InvViewProj);
	vec3    surfacePosition = DepthToWorldPos(depthValue, screenTexCoord, invViewProj);

	mat4	userToLHZ = GET_CONSTANT(SceneInfo, UserToLHZ);
	mat4 	inverseDecalTransform = mul(userToLHZ, fInput.fragInverseDecalTransform);
	vec3 	normalizedDecalSpace = mul(inverseDecalTransform, vec4(surfacePosition, 1.0)).xyz;
	
	if (ANY_BOOL(VEC_GREATER_THAN(normalizedDecalSpace, VEC3_ONE)) || ANY_BOOL(VEC_LESS_THAN(normalizedDecalSpace, -VEC3_ONE)))
		discard;

	vec2	fragUV0 = normalizedDecalSpace.xy * vec2(0.5f, 0.5f) + 0.5f;

#if 	defined(HAS_Atlas)
	vec2	fragUV1 = fragUV0;
	
	fragUV0 *= LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID)) * 4 + 1)).xy;
	fragUV0 += LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID)) * 4 + 1)).zw;
	fragUV1 *= LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID) + 1) * 4 + 1)).xy;
	fragUV1 += LOADF4(GET_RAW_BUFFER(Atlas), RAW_BUFFER_INDEX(min(LOADU(GET_RAW_BUFFER(Atlas), 0) - 1, uint(fInput.fragAtlas_TextureID) + 1) * 4 + 1)).zw;
	
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
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
#endif

	float	fadeRangeTop = GET_CONSTANT(Material, Decal_FadeTop);
	float 	startFadeTop = 1.0f - fadeRangeTop;
	float	fadeRangeBottom = GET_CONSTANT(Material, Decal_FadeBottom);
	float 	startFadeBottom = 1.0f - fadeRangeBottom;

	float 	startFade = (normalizedDecalSpace.z < 0.0f) ? startFadeBottom : startFadeTop;
	float 	fadeRange = (normalizedDecalSpace.z < 0.0f) ? fadeRangeBottom : fadeRangeTop;
	float	fade = 1.0f - clamp((abs(normalizedDecalSpace.z) - startFade) / fadeRange, 0.0f, 1.0f);

#if 	defined(HAS_Diffuse)
	vec4 	diffuseColor = SAMPLE(Diffuse_DiffuseMap, fragUV0);
	
#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    diffuseColor2 = SAMPLE(Diffuse_DiffuseMap, fragUV1);
		diffuseColor = mix(diffuseColor, diffuseColor2, blendMix);
	}
#	endif

	diffuseColor *= fInput.fragDiffuse_Color;

	diffuseColor.a *= fade;
	diffuseColor.rgb *= diffuseColor.a;
	fOutput.Output0 = diffuseColor;
#else
	// Diffuse buffer
	fOutput.Output0 = vec4(0.0, 0.0, 0.0, 0.0);
#endif

#if 	defined(HAS_Emissive)

	vec4 	emissiveColor = SAMPLE(Emissive_EmissiveMap, fragUV0);
	
#   if defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    emissiveColor2 = SAMPLE(Emissive_EmissiveMap, fragUV1);
		emissiveColor = mix(emissiveColor, emissiveColor2, blendMix);
	}
#	endif

	emissiveColor *= vec4(fInput.fragEmissive_EmissiveColor, fade);

	emissiveColor.rgb *= emissiveColor.a;
	emissiveColor.a = 0.0f; // Not absorptive
	fOutput.Output1 = emissiveColor;
#else
	// Emissive buffer
	fOutput.Output1 = vec4(0.0, 0.0, 0.0, 0.0);
#endif
	// Normal / roughness / metalness
	fOutput.Output2 = vec4(0.0, 0.0, 0.0, 0.0);
}
