#pragma once

// Render passes
#if defined(ParticlePass_Distortion)
#	define	PK_FORWARD_DISTORTION_PASS
#elif defined(ParticlePass_Tint)
#	define	PK_FORWARD_TINT_PASS
#elif defined(ParticlePass_TransparentPostDisto) || defined(ParticlePass_Transparent)
#	define	PK_FORWARD_COLOR_PASS
#elif defined(ParticlePass_Opaque)
#	define	PK_DEFERRED_COLOR_PASS
#elif defined(ParticlePass_Decal)
#	define	PK_DEFERRED_DECAL_PASS
#elif defined(ParticlePass_Lighting)
#	define	PK_LIGHTING_PASS
#else
#	error "Unrecognized particle render pass"
#endif

// Geometry data for the current fragment (directly coming from vertex shader)
struct	SFragGeometry
{
	vec2	m_UV0;
	vec2	m_UV1;
	float	m_BlendMix; // Lerp ratio between UV0 color and UV1 color
	vec3	m_Normal;
	vec4	m_Tangent;

#	if defined(HAS_GeometryDecal)
	vec3	m_NormalizedDecalSpace;
#	endif

#	if defined(HAS_TransformUVs)
	// Used by TransformUVs to sample alpha and color separately:
	vec2	m_AlphaUV0;
	vec2	m_AlphaUV1;
	bool	m_UseAlphaUVs;

	// We use m_RotateTangent to store the inverse UV rotation:
	// we can then rotate the normal in UV space using this instead of rotating the 3D tangent
	mat2	m_TangentRotation;
#	endif
};

// Surface data, used to compute the final color
#if defined(PK_FORWARD_DISTORTION_PASS)
struct	SFragSurface
{
	vec3	m_Distortion;
};
#elif defined(PK_FORWARD_TINT_PASS)
struct	SFragSurface
{
	vec3	m_Tint;
};
#elif defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
struct	SFragSurface
{
	vec3	m_Emissive;
	vec4	m_Diffuse;
	vec3	m_Normal;
	float	m_Depth;
	float	m_Roughness;
	float	m_Metalness;
};
#elif defined(PK_DEFERRED_DECAL_PASS)
struct	SFragSurface
{
	vec3	m_Emissive;
	vec4	m_Diffuse;
	vec3	m_Normal;
	float	m_Roughness;
	float	m_Metalness;
};
#elif defined(PK_LIGHTING_PASS)
struct	SFragSurface
{
	// Surface info:
	vec3	m_Diffuse;
	vec3	m_WorldPosition;
	vec3	m_Normal;
	float	m_Roughness;
	float	m_Metalness;

	// Lighting output color:
	vec3	m_LightAccu;
};
#else
#	error "Unrecognized particle render pass"
#endif

vec3	DepthToWorldPos(float depthValue, vec2 uv, mat4 invViewProj)
{
	vec2	clipSpace = uv * 2.0 - 1.0;
	vec4	projPos = vec4(clipSpace, depthValue, 1.0);
	projPos = mul(invViewProj, projPos);
	return projPos.xyz / projPos.w;
}

#if !defined(PK_LIGHTING_PASS)

void	FragmentInputToFragGeometry(IN(SFragmentInput) fInput, OUT(SFragGeometry) fGeom FS_ARGS)
{
#	if defined(HAS_GeometryDecal)

	// For decals, we need to compute the UVs in the fragment shader:
	vec2	screenTexCoord = fInput.fragViewProjPosition.xy / fInput.fragViewProjPosition.w * 0.5 + 0.5;
	float	depthValue = SAMPLE(DepthSampler, screenTexCoord).r;
	mat4	invViewProj = GET_CONSTANT(SceneInfo, InvViewProj);
	vec3	surfacePosition = DepthToWorldPos(depthValue, screenTexCoord, invViewProj);
	mat4	userToLHZ = GET_CONSTANT(SceneInfo, UserToLHZ);
	mat4	inverseDecalTransform = mul(userToLHZ, fInput.fragInverseDecalTransform);

	fGeom.m_NormalizedDecalSpace = mul(inverseDecalTransform, vec4(surfacePosition, 1.0)).xyz;

	if (ANY_BOOL(VEC_GREATER_THAN(fGeom.m_NormalizedDecalSpace, VEC3_ONE)) ||
		ANY_BOOL(VEC_LESS_THAN(fGeom.m_NormalizedDecalSpace, -VEC3_ONE)))
		discard;

	fGeom.m_UV0 = fGeom.m_NormalizedDecalSpace.xy * vec2(0.5f, 0.5f) + 0.5f;

// For decals, adjust UVs if the feature atlas is enabled
// (this is done in the billboarding tasks for billboard and ribbons):
#		if defined(HAS_Atlas)
	fGeom.m_UV1 = fGeom.m_UV0;
	fGeom.m_UV0 *= fInput.fragRect0.xy;
	fGeom.m_UV0 += fInput.fragRect0.zw;
	fGeom.m_UV1 *= fInput.fragRect1.xy;
	fGeom.m_UV1 += fInput.fragRect1.zw;
#			if !defined(FINPUT_fragRect0) || !defined(FINPUT_fragRect1)
#				error "Need atlas sub rectangles as fragment shader input"
#			endif
#		else
	fGeom.m_UV1 = VEC2_ZERO;
#		endif

#	else
	// Here UVs are computed in the vertex shader, we can just copy the geometry values:
#		if defined(FINPUT_fragUV0) // Particle has UV0
	fGeom.m_UV0 = fInput.fragUV0;
#		else
	fGeom.m_UV0 = VEC2_ZERO;
#		endif
#		if defined(FINPUT_fragUV1) // Particle has UV1
	fGeom.m_UV1 = fInput.fragUV1;
#		else
	fGeom.m_UV1 = VEC2_ZERO;
#		endif
#	endif

#	if defined(FINPUT_fragNormal) // Particle has normal
	fGeom.m_Normal = fInput.fragNormal;
#	else
	fGeom.m_Normal = VEC3_ZERO;
#	endif
#	if defined(FINPUT_fragTangent) // Particle has tangent
	fGeom.m_Tangent = fInput.fragTangent;
#	else
	fGeom.m_Tangent = VEC4_ZERO;
#	endif
#	if defined(FINPUT_fragAtlas_TextureID) // Particle has atlas
	fGeom.m_BlendMix = fract(fInput.fragAtlas_TextureID);
#	else
	fGeom.m_BlendMix = 0.0f;
#	endif

#	if defined(HAS_TransformUVs)
	fGeom.m_AlphaUV0 = fGeom.m_UV0;
	fGeom.m_AlphaUV1 = fGeom.m_UV1;
	fGeom.m_UseAlphaUVs = false;
	fGeom.m_TangentRotation = BUILD_MAT2(vec2(1, 0), vec2(0, 1)); // Identity
#	endif
}

void	FragGeometryToFragSurface(IN(SFragGeometry) fGeom, OUT(SFragSurface) fSurf, float fragDepth FS_ARGS)
{
#	if defined(PK_FORWARD_DISTORTION_PASS)
	fSurf.m_Distortion = VEC3_ZERO;
#	elif defined(PK_FORWARD_TINT_PASS)
	fSurf.m_Tint = VEC3_ZERO;
#	elif defined(PK_FORWARD_COLOR_PASS) || defined(PK_DEFERRED_COLOR_PASS)
	fSurf.m_Emissive = VEC3_ZERO;
	fSurf.m_Diffuse = VEC4_ZERO;
	fSurf.m_Normal = fGeom.m_Normal;
	fSurf.m_Depth = fragDepth;
	fSurf.m_Roughness = 0.0f;
	fSurf.m_Metalness = 0.0f;
#	elif defined(PK_DEFERRED_DECAL_PASS)
	fSurf.m_Emissive = VEC3_ZERO;
	fSurf.m_Diffuse = VEC4_ZERO;
	fSurf.m_Normal = fGeom.m_Normal;
	fSurf.m_Roughness = 0.0f;
	fSurf.m_Metalness = 0.0f;
#	else
#		error "Unrecognized particle render pass"
#	endif
}

#else

vec3	UnpackNormalSpheremap(vec2 normal FS_ARGS)
{
	vec2	fenc = normal * 4.0f - 2.0f;
	float	f = dot(fenc, fenc);
	float	g = sqrt(1.0f - f / 4.0f);
	vec3	outNormal;
	outNormal.xy = fenc * g;
	outNormal.z = 1.0f - f / 2.0f;
	return mul(GET_CONSTANT(SceneInfo, UnpackNormalView), vec4(outNormal, 0.0f)).xyz;
}

void	GBufferToFragSurface(OUT(SFragSurface) fSurf, vec3 clipPos FS_ARGS)
{
	mat4	invViewProj = GET_CONSTANT(SceneInfo, InvViewProj);
	vec2	screenTexCoord = clipPos.xy * 0.5f + 0.5f;
	vec4	packedNormalRoughMetal = SAMPLE(NormalRoughMetalSampler, screenTexCoord);
	float	depthValue = SAMPLE(DepthSampler, screenTexCoord).r;

	fSurf.m_Diffuse = SAMPLE(DiffuseSampler, screenTexCoord).rgb;
	fSurf.m_WorldPosition = DepthToWorldPos(depthValue, screenTexCoord, invViewProj);
	fSurf.m_Normal = UnpackNormalSpheremap(packedNormalRoughMetal.rg FS_PARAMS);
	fSurf.m_Roughness = packedNormalRoughMetal.z;
	fSurf.m_Metalness = packedNormalRoughMetal.w;
	fSurf.m_LightAccu = VEC3_ZERO;
}

#endif
