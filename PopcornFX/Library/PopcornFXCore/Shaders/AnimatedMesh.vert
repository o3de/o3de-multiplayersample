
struct 	SBoneTransform
{
	vec4 	m_Rotation;
	vec3 	m_Position;
	vec3	m_Scale;
};

#if 	defined(HAS_SkeletalAnimationUseBonesScale)
#	define PIXELS_PER_BONE 	3.0f
#else
#	define PIXELS_PER_BONE 	2.0f
#endif

SBoneTransform  LerpBoneTransforms(IN(SBoneTransform) a, IN(SBoneTransform) b, float v)
{
	SBoneTransform 	c;
	vec4 			rotationB = b.m_Rotation;

	if (dot(a.m_Rotation, b.m_Rotation) < 0.0f)
		rotationB *= -1.0f;
	c.m_Rotation = normalize(mix(a.m_Rotation, rotationB, v));
	c.m_Position = mix(a.m_Position, b.m_Position, v);
	c.m_Scale = mix(a.m_Scale, b.m_Scale, v);
	return c;
}

vec3 RemapPositionsFromUnormToBounds(vec3 value VS_ARGS)
{
	vec3 	minBounds = GET_CONSTANT(Material, SkeletalAnimation_AnimPositionsBoundsMin);
	vec3 	maxBounds = GET_CONSTANT(Material, SkeletalAnimation_AnimPositionsBoundsMax);
	vec3 	range = maxBounds - minBounds;
	return minBounds + value * range;
}

#if 	defined(HAS_SkeletalAnimationUseBonesScale)
vec3 RemapScalesFromUnormToBounds(vec3 value VS_ARGS)
{
	vec3 	minBounds = GET_CONSTANT(Material, SkeletalAnimationUseBonesScale_AnimScalesBoundsMin);
	vec3 	maxBounds = GET_CONSTANT(Material, SkeletalAnimationUseBonesScale_AnimScalesBoundsMax);
	vec3 	range = maxBounds - minBounds;
	return minBounds + value * range;
}
#endif

SBoneTransform 	ReadBoneTransforms(int animationId, float boneId, float animationCursor VS_ARGS)
{
	// Texture info:
	vec2 animTexResol = vec2(GET_CONSTANT(Material, SkeletalAnimation_AnimTextureResolution));
	vec2 pxlUvSize = 1.0f / animTexResol;
	vec2 offsetToBottomPixel = vec2(0.0f, pxlUvSize.y);
	// Remap anim cursor to start the sampling in the middle of the first pixel and finish in the middle of the last one:
	float animCursorToUvRange = animationCursor * (1.0f - pxlUvSize.x);
	float animCursorToPrevPixel = mod(animCursorToUvRange, pxlUvSize.x);
	float snappedAnimCursor = (animCursorToUvRange - animCursorToPrevPixel) + pxlUvSize.x * 0.5f;
	// UV offset to read from correct animation index:
	float animCount = float(GET_CONSTANT(Material, SkeletalAnimation_AnimTracksCount));
	float animationUVOffset = ((animTexResol.y / animCount) * pxlUvSize.y) * float(animationId);
	// Start pixel to read from:
	vec2 animUV = vec2(snappedAnimCursor, boneId * PIXELS_PER_BONE * pxlUvSize.y + animationUVOffset);

	SBoneTransform	boneTransform;

	// Sample the texture:
	vec4 position = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV + offsetToBottomPixel * 0.5f, 0);
	vec4 rotation = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV + offsetToBottomPixel * 1.5f, 0);
	boneTransform.m_Rotation = rotation * 2.0f - 1.0f;
	boneTransform.m_Position = RemapPositionsFromUnormToBounds(position.xyz VS_PARAMS);
#if 	defined(HAS_SkeletalAnimationUseBonesScale)
	vec4 scale = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV + offsetToBottomPixel * 2.5f, 0);
	boneTransform.m_Scale = RemapScalesFromUnormToBounds(scale.xyz VS_PARAMS);
#else
	boneTransform.m_Scale = vec3(1.0f, 1.0f, 1.0f);
#endif

#if 	defined(HAS_SkeletalAnimationInterpolate)
	SBoneTransform	boneTransform1;

	float snappedAnimCursor1 = (animCursorToUvRange + (pxlUvSize.x - animCursorToPrevPixel)) + pxlUvSize.x * 0.5f;
	float currentLerpRatio = animCursorToPrevPixel / pxlUvSize.x;
	vec2 animUV1 = vec2(snappedAnimCursor1, float(boneId) * PIXELS_PER_BONE * pxlUvSize.y + animationUVOffset);

	// Sample the texture:
	vec4 position1 = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV1 + offsetToBottomPixel * 0.5f, 0);
	vec4 rotation1 = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV1 + offsetToBottomPixel * 1.5f, 0);
	boneTransform1.m_Rotation = rotation1 * 2.0f - 1.0f;
	boneTransform1.m_Position = RemapPositionsFromUnormToBounds(position1.xyz VS_PARAMS);
#if 	defined(HAS_SkeletalAnimationUseBonesScale)
	vec4 scale1 = SAMPLELOD(SkeletalAnimation_AnimationTexture, animUV1 + offsetToBottomPixel * 2.5f, 0);
	boneTransform1.m_Scale = RemapScalesFromUnormToBounds(scale1.xyz VS_PARAMS);
#else
	boneTransform1.m_Scale = vec3(1.0f, 1.0f, 1.0f);
#endif
	boneTransform = LerpBoneTransforms(boneTransform, boneTransform1, currentLerpRatio);
#endif

	return boneTransform;
}

float RemapValue(float value, float min1, float max1, float min2, float max2)
{
	float 	value_01 = (value - min1) / (max1 - min1);
	return value_01 * (max2 - min2) + min2;
}

mat4 QuatToMat4(vec4 quat)
{
    mat4 m = mat4(VEC4_ZERO, VEC4_ZERO, VEC4_ZERO, VEC4_ZERO);
    float x = quat.x, y = quat.y, z = quat.z, w = quat.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    m[0][0] = 1.0 - (yy + zz);
    m[1][0] = xy - wz;
    m[2][0] = xz + wy;

    m[0][1] = xy + wz;
    m[1][1] = 1.0 - (xx + zz);
    m[2][1] = yz - wx;

    m[0][2] = xz - wy;
    m[1][2] = yz + wx;
    m[2][2] = 1.0 - (xx + yy);

    m[3][3] = 1.0;
    return m;
}

mat4  	BoneTransformToMatrix(SBoneTransform tr)
{
	mat4 res = QuatToMat4(tr.m_Rotation);
	res[3] = vec4(tr.m_Position, 1.0f);
	res[0].xyz *= tr.m_Scale;
	res[1].xyz *= tr.m_Scale;
	res[2].xyz *= tr.m_Scale;	
	return BUILD_MAT4(res[0], res[1], res[2], res[3]);
}

mat4 	LerpMatrix(mat4 m0, mat4 m1, float ratio)
{
	mat4 	res;
	
	res[0] = mix(m0[0], m1[0], ratio);
	res[1] = mix(m0[1], m1[1], ratio);
	res[2] = mix(m0[2], m1[2], ratio);
	res[3] = mix(m0[3], m1[3], ratio);
	return res;
}

void    VertexMain(IN(SVertexInput) vInput, INOUT(SVertexOutput) vOutput VS_ARGS)
{
	mat4 	meshTransform = GetMeshMatrix(vInput VS_PARAMS);
	
#if 	defined(VOUTPUT_fragSkeletalAnimation_CurrentAnimTrack) && defined(VOUTPUT_fragSkeletalAnimation_AnimationCursor) && defined(VINPUT_BoneIds)
	mat4 animTr = BUILD_MAT4(VEC4_ZERO, VEC4_ZERO, VEC4_ZERO, VEC4_ZERO);

	int boneIdx = 0;
	while (boneIdx < 4)
	{
		SBoneTransform boneTr = ReadBoneTransforms(	vOutput.fragSkeletalAnimation_CurrentAnimTrack,
													vInput.BoneIds[boneIdx],
													vOutput.fragSkeletalAnimation_AnimationCursor
													VS_PARAMS);
		mat4 boneMat = BoneTransformToMatrix(boneTr);
#	if defined(HAS_SkeletalAnimationInterpolateTracks)
		SBoneTransform boneTr2 = ReadBoneTransforms(vOutput.fragSkeletalAnimationInterpolateTracks_NextAnimTrack,
													vInput.BoneIds[boneIdx],
													vOutput.fragSkeletalAnimationInterpolateTracks_NextAnimationCursor
													VS_PARAMS);
		mat4 boneMat2 = BoneTransformToMatrix(boneTr2);
		boneMat = LerpMatrix(boneMat, boneMat2, vOutput.fragSkeletalAnimationInterpolateTracks_TransitionRatio);
#	endif
		animTr[0] += boneMat[0] * vInput.BoneWeights[boneIdx];
		animTr[1] += boneMat[1] * vInput.BoneWeights[boneIdx];
		animTr[2] += boneMat[2] * vInput.BoneWeights[boneIdx];
		animTr[3] += boneMat[3] * vInput.BoneWeights[boneIdx];
		++boneIdx;
	}

	mat4 finalTransform = animTr;
	mat4 user2lhz = GET_CONSTANT(SceneInfo, UserToLHZ);
	mat4 lhz2user = GET_CONSTANT(SceneInfo, LHZToUser);
	finalTransform = mul(lhz2user, finalTransform);
	finalTransform = mul(finalTransform, user2lhz);
	finalTransform = mul(meshTransform, finalTransform);
	vec3 worldPos = mul(finalTransform, vec4(vInput.Position, 1.0f)).xyz;

#	if 	defined(HAS_SkeletalAnimationUseBonesScale)
	mat4 transformNoScale = BUILD_MAT4(	normalize(GET_MATRIX_X_AXIS(finalTransform)),
										normalize(GET_MATRIX_Y_AXIS(finalTransform)),
										normalize(GET_MATRIX_Z_AXIS(finalTransform)),
										GET_MATRIX_W_AXIS(finalTransform));
#	else
	mat4 transformNoScale = finalTransform;
#	endif

#	if	defined(VINPUT_Normal)
	vOutput.fragNormal = mul(transformNoScale, vec4(vInput.Normal.xyz, 0)).xyz;
#	endif
#	if	defined(VINPUT_Tangent)
	vOutput.fragTangent = vec4(mul(transformNoScale, vec4(vInput.Tangent.xyz, 0)).xyz, vInput.Tangent.w);
#	endif

#else
	vec3 worldPos = mul(meshTransform, vec4(vInput.Position.xyz, 1.0f)).xyz;
#endif

	vOutput.VertexPosition = mul(GET_CONSTANT(SceneInfo, ViewProj), vec4(worldPos, 1.0f));

#if	defined(VOUTPUT_fragWorldPosition)
	vOutput.fragWorldPosition = worldPos;
#endif
#if	defined(VOUTPUT_fragViewProjPosition)
	vOutput.fragViewProjPosition = vOutput.VertexPosition;
#endif
#if	defined(VINPUT_Enabled)
	if (vInput.Enabled == 0u)
		vOutput.geomSize = 0.f;
#endif
}
