
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

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	vec4    color;

#if     defined(FINPUT_fragDistortion_Color)
	color = fInput.fragDistortion_Color;
#else
	color = vec4(1., 1., 1., 1.);
#endif

#if     defined(HAS_SoftParticles) || defined(HAS_Distortion)
	vec4    projPos = fInput.fragViewProjPosition;
	float   rcpw = 1.0 / projPos.w;
	float   fragDepth_cs = projPos.z * rcpw;
	//  if (IsDepthReversed())
	//      fragDepth_cs = 1.0 - fragDepth_cs;
#endif

#if     defined(HAS_Distortion)
	vec4    textureColor;
	
	color *= clipDepthToDistortionFade(fragDepth_cs  FS_PARAMS);

#   if      defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	
	if (blendingType == 2)
	{
		// Motion vectors
		vec2	scale = GET_CONSTANT(Material, Atlas_DistortionStrength);
		vec2	curVectors = ((SAMPLE(Atlas_MotionVectorsMap, fInput.fragUV0).rg * 2.0f) - 1.0f) * scale;
		vec2	nextVectors = ((SAMPLE(Atlas_MotionVectorsMap, fInput.fragUV1).rg * 2.0f) - 1.0f) * scale;
		float	cursor = fract(fInput.fragAtlas_TextureID);
		
		curVectors *= cursor;
		nextVectors *= (1.0f - cursor);
		vec2	uv0 = fInput.fragUV0 - curVectors;
		vec2	uv1 = fInput.fragUV1 + nextVectors;
		
		vec4	textureColor1 = SAMPLE(Distortion_DistortionMap, uv0);
		vec4	textureColor2 = SAMPLE(Distortion_DistortionMap, uv1);
		textureColor = mix(textureColor1, textureColor2, cursor);
	}
	else
	{
		textureColor = SAMPLE(Distortion_DistortionMap, fInput.fragUV0);
	
		if (blendingType == 1)
		{
			// Linear
			vec4    textureColor2 = SAMPLE(Distortion_DistortionMap, fInput.fragUV1);
			textureColor = mix(textureColor, textureColor2, fract(fInput.fragAtlas_TextureID));
		}
	}
#	else
	// No atlas
	textureColor = SAMPLE(Distortion_DistortionMap, fInput.fragUV0);
#	endif // defined(HAS_Atlas)

	color *= textureColor * vec4(2., 2., 1., 0.) - vec4(1.00392, 1.00392, 0., -1.); // 128 is considered as the mid-value (in range 0-255)
#endif

#if     defined(HAS_SoftParticles)
	vec2    screenUV = projPos.xy * rcpw * vec2(0.5, 0.5) + 0.5;
	float   sceneDepth_cs = SAMPLE(DepthSampler, screenUV).x;
	float   sceneDepth = clipToLinearDepth(sceneDepth_cs FS_PARAMS);
	float   fragDepth = clipToLinearDepth(fragDepth_cs FS_PARAMS);
	float   invSoftnessDistance = 1.0f / GET_CONSTANT(Material, SoftParticles_SoftnessDistance);
	float   depthfade = clamp((sceneDepth - fragDepth) * invSoftnessDistance, 0.f, 1.f);
	color *= depthfade;
#endif

	fOutput.Output0 = color;
}
