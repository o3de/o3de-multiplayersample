#if 	defined(HAS_TransformUVs)
vec2 transformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}
#endif

#if defined(HAS_SoftParticles)
float   clipToLinearDepth(float depth FS_ARGS)
{
	float zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	return (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
}
#endif

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec4    color = vec4(0,0,0,0);

#if     defined(FINPUT_fragDiffuse_Color)
	color = fInput.fragDiffuse_Color;
#else
	color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
#endif

#if	    defined(FINPUT_fragColor0)
	color *= fInput.fragColor0;
#endif

vec2	fragUV0 = fInput.fragUV0;

#if     defined(HAS_Diffuse) || defined(HAS_Emissive) // has texture sampling

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

#if	defined(HAS_Emissive)
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

#if	defined(HAS_EmissiveRamp)
	emissiveColor1 = SAMPLE(EmissiveRamp_RampMap, vec2(emissiveColor1.x,0.0)).rgb;
#endif

	emissiveColor1 *= fInput.fragEmissive_EmissiveColor;
	color.rgb += emissiveColor1.rgb;
#endif

	fOutput.Output0 = color;
}
