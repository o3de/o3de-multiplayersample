
vec2 PackNormalSpheremap(vec3 normal FS_ARGS)
{
	normal = normalize(normal);
	normal = mul(GET_CONSTANT(SceneInfo, PackNormalView), vec4(normal, 0.0f)).xyz;
	float f = sqrt(8.0f * normal.z + 8.0f);
	return normal.xy / f + 0.5f;
}

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
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

#if     defined(HAS_Diffuse)
	vec4	textureColor1 = SAMPLE(Diffuse_DiffuseMap, fInput.fragUV0);
	color *= textureColor1;
#endif

#if defined(HAS_Opaque)
    if (GET_CONSTANT(Material, Opaque_Type) == 1 &&
		color.a < GET_CONSTANT(Material, Opaque_MaskThreshold))
        discard;
#endif

#if     defined(HAS_Lit)
	vec3    normalTex =  SAMPLE(Lit_NormalMap, fInput.fragUV0).xyz;
	normalTex = 2.0f * normalTex.xyz - vec3(1.0f, 1.0f, 1.0f);

	vec3    T = normalize(fInput.fragTangent.xyz);
	vec3    N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T) * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
	N = fInput.IsFrontFace ? N : -N;
	mat3    TBN = BUILD_MAT3(T, B, N);

	normal = normalize(mul(TBN, normalTex));

	vec2    roughMetal = SAMPLE(Lit_RoughMetalMap, fInput.fragUV0).xy;

	roughness = GET_CONSTANT(Material, Lit_Roughness);
	roughness *= roughMetal.x;
	metalness = GET_CONSTANT(Material, Lit_Metalness);
	metalness *= roughMetal.y;
#endif

	vec2 	packedNormal = PackNormalSpheremap(normal FS_PARAMS);
	vec4	normalSpec = vec4(packedNormal, roughness, metalness);

	fOutput.Output0 = color;
    fOutput.Output1 = fInput.fragViewProjPosition.z / fInput.fragViewProjPosition.w;
    fOutput.Output2 = VEC4_ZERO;
    fOutput.Output3 = normalSpec;
}
