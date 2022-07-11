
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
	color *= clipDepthToDistortionFade(fragDepth_cs  FS_PARAMS);

	vec2    fragUV0 = applyCorrectDeformation(fInput, fInput.fragUV0 FS_PARAMS);

#   if      defined(HAS_Atlas)
	int		blendingType = GET_CONSTANT(Material, Atlas_Blending);
	vec2    fragUV1 = applyCorrectDeformation(fInput, fInput.fragUV1 FS_PARAMS);

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

	vec4    textureColor = SAMPLE(Distortion_DistortionMap, fragUV0);

#   if      defined(HAS_Atlas)
	if (blendingType >= 1)
	{
		vec4    textureColor2 = SAMPLE(Distortion_DistortionMap, fragUV1);
		textureColor = mix(textureColor, textureColor2, blendMix);
	}
#	endif


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
