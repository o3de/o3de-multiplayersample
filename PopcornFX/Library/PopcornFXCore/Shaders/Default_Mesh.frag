
vec2 PackNormalSpheremap(vec3 normal FS_ARGS)
{
	normal = normalize(normal);
	normal = mul(GET_CONSTANT(SceneInfo, PackNormalView), vec4(normal, 0.0f)).xyz;
	float f = sqrt(8.0f * normal.z + 8.0f);
	return normal.xy / f + 0.5f;
}

#if 	defined(HAS_TransformUVs)
vec2 transformUV(vec2 UV, vec2 scale, mat2 rotation, vec2 offset)
{
	return mul(rotation, UV * scale) + offset;
}
#endif

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	float   roughness = -1.0f;
	float   metalness = -1.0f;
	vec3    normal = vec3(1,0,0);
	vec4    color = vec4(0,0,0,0);
	vec4    emissive = vec4(0,0,0,0);

#if	    defined(FINPUT_fragDiffuse_Color)
	color = fInput.fragDiffuse_Color;
#else
	color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
#endif

#if	    defined(FINPUT_fragColor0)
	color *= fInput.fragColor0;
#endif

vec2 fragUV0 = fInput.fragUV0;

#if		defined(HAS_TransformUVs)
	float	sinR = sin(fInput.fragTransformUVs_UVRotate);
	float	cosR = cos(fInput.fragTransformUVs_UVRotate);
	mat2	UVRotation = mat2(cosR, sinR, -sinR, cosR);
	vec2	UVScale = fInput.fragTransformUVs_UVScale;
	vec2	UVOffset = fInput.fragTransformUVs_UVOffset;
	vec2	oldFragUV0 = fragUV0;	
	fragUV0 = transformUV(fragUV0, UVScale, UVRotation, UVOffset); // scale then rotate then translate UV
	fragUV0 = fract(fragUV0);
	bool	RGBOnly = GET_CONSTANT(Material, TransformUVs_RGBOnly) != 0;
#endif

#if	defined(HAS_Diffuse)
	vec4    textureColor = SAMPLE(Diffuse_DiffuseMap, fragUV0);
	
#	if defined(HAS_TransformUVs)
		if (RGBOnly)
		{
			textureColor.a = SAMPLE(Diffuse_DiffuseMap, oldFragUV0).a;
		}
#	endif

#if defined(HAS_DiffuseRamp)
	textureColor.rgb =  SAMPLE(DiffuseRamp_RampMap, vec2(textureColor.x,0.0)).rgb;
#endif

	color *= textureColor;

#if	defined(HAS_AlphaRemap)
	vec2    alphaTexCoord = vec2(color.a, fInput.fragAlphaRemap_Cursor);
	color.a = SAMPLE(AlphaRemap_AlphaMap, alphaTexCoord).r;
#endif 

#endif // defined HAS_Diffuse

#if	defined(HAS_Opaque)
    if (GET_CONSTANT(Material, Opaque_Type) == 1 &&
		color.a < GET_CONSTANT(Material, Opaque_MaskThreshold))
        discard;
#endif

#if defined(HAS_DiffuseRamp)
	color.rgb = SAMPLE(DiffuseRamp_RampMap, vec2(color.r, 0.0)).rgb;
#endif

#if     defined(HAS_Lit)

	vec3    normalTex =  SAMPLE(Lit_NormalMap, fragUV0).xyz;

	normalTex = 2.0f * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);
	
// if uv are rotated, inverse rotate the billboard space normal
// to cancel tangent-UV mismatch
#	if	defined(HAS_TransformUVs)
		float sinMR = -sinR; // sin(-rot) = -sin(rot)
		float cosMR = cosR; // cos(-rot) = cos(rot)
		mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
		normalTex.xy = mul(UVInverseRotation, normalTex.xy);
#	endif

	vec3	T = normalize(fInput.fragTangent.xyz);
	vec3	N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3	TBN = BUILD_MAT3(T, B, N);

	normal = normalize(mul(TBN, normalTex));

	vec2    roughMetal = SAMPLE(Lit_RoughMetalMap, fragUV0).xy;

	roughness = GET_CONSTANT(Material, Lit_Roughness);
	roughness *= roughMetal.x;
	metalness = GET_CONSTANT(Material, Lit_Metalness);
	metalness *= roughMetal.y;

#elif   defined(HAS_LegacyLit)

	vec3    normalTex =  SAMPLE(LegacyLit_NormalMap, fragUV0).xyz;
	normalTex = 2.0f * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);
	
#	if	defined(HAS_TransformUVs)
		float sinMR = -sinR; // sin(-rot) = -sin(rot)
		float cosMR = cosR; // cos(-rot) = cos(rot)
		mat2 UVInverseRotation = mat2(cosMR, sinMR, -sinMR, cosMR);
		normalTex.xy = mul(UVInverseRotation, normalTex.xy);
#	endif
	
	vec3	T = normalize(fInput.fragTangent.xyz);
	vec3	N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3	TBN = BUILD_MAT3(T, B, N);
	normal = normalize(mul(TBN, normalTex));

	// In the old lighting feature, we stored the specular and glossiness in the specular map.
	// To mimic something similar, we juste set the roughness to (1.0 - specularValue) * 0.7 + 0.3 to avoid very sharp specular which did not exist before
	// The metalness is always 0.
	roughness = (1.0 - SAMPLE(LegacyLit_SpecularMap, fragUV0).x) * 0.7 + 0.3;
	metalness = 0.0f;

#else

	emissive = color;
	color = vec4(0,0,0,0);

#endif

#if defined(HAS_Emissive)

	vec3 emissiveColor1 = SAMPLE(Emissive_EmissiveMap, fragUV0).rgb;	
	#if defined(HAS_EmissiveRamp)
		emissiveColor1 = SAMPLE(EmissiveRamp_RampMap, vec2(emissiveColor1.x,0.0)).rgb;
	#endif
	emissive.rgb += emissiveColor1 * fInput.fragEmissive_EmissiveColor;

#endif

	vec4    normalSpec;
	normalSpec = vec4(PackNormalSpheremap(normal FS_PARAMS), roughness, metalness);

	fOutput.Output0 = color;
	fOutput.Output1 = fInput.fragViewProjPosition.z / fInput.fragViewProjPosition.w;
	fOutput.Output2 = emissive;
	fOutput.Output3 = normalSpec;
}
