
#if defined(HAS_SoftParticles)
float   clipToLinearDepth(float depth FS_ARGS)
{
	float zNear = GET_CONSTANT(SceneInfo, ZBufferLimits).x;
	float zFar = GET_CONSTANT(SceneInfo, ZBufferLimits).y;
	return (-zNear * zFar) / (depth * (zFar - zNear) - zFar);
}
#endif

#if defined(HAS_Lit)

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

vec3	computeBRDF(vec3 surfToLight, vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec3 surfaceColor FS_ARGS)
{
	vec3    halfVec = normalize(surfToLight + surfToView);

	float   NoL = max(0.0f, dot(surfToLight, surfaceNormal));

#if     defined(HAS_NormalWrap)
	float   normalWrapFactor = GET_CONSTANT(Material, NormalWrap_WrapFactor);
	NoL = normalWrapFactor + (1.0f - normalWrapFactor) * NoL;
#endif

	float   specIntensity = max(0.0f, dot(halfVec, surfaceNormal));
	float   NoV = max(EPSILON, dot(surfToView, surfaceNormal)); // Weird behavior when this is near 0

	vec3    surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceColor, metalness);

	vec3    F  = fresnelSchlick(surfaceMetalColor, max(0.0f, dot(halfVec, surfToView)));
	float   D = normalDistFuncGGX(specIntensity, roughness);
	float   G = schlickGGX(NoL, NoV, roughness);

	vec3 diffuseBRDF = mix(vec3(1.0, 1.0, 1.0) - F, vec3(0.0, 0.0, 0.0), metalness) * surfaceColor;
	vec3 specularBRDF = (F * D * G) / max(EPSILON, 4.0 * NoL * NoV);

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

vec3	computeAmbientBRDF(vec3 surfToView, vec3 surfaceNormal, float roughness, float metalness, vec3 surfaceColor FS_ARGS)
{
	// Cubemaps are expected to be cosined filtered up to the 7th mipmap which should contain the fully diffuse 180 degrees cosine-filtered cubemap.
	const float	kCubemapMipRange = 6;

	vec3	surfaceMetalColor = mix(METAL_FRESNEL_FACTOR, surfaceColor, metalness);
	vec3	kS = fresnelSchlickRoughness(max(dot(surfaceNormal, surfToView), 0.0), surfaceMetalColor, roughness);
	vec3	kD = 1.0f - kS;
	kD *= 1.0f - metalness;

	// Ambient diffuse
	vec3	irradiance = SAMPLELOD_CUBE(EnvironmentMapSampler, toCubemapSpace(surfaceNormal FS_PARAMS), kCubemapMipRange).rgb;
	vec3	diffuse = irradiance * surfaceColor;

	// Ambient specular
	vec3	sampleDir = toCubemapSpace(reflect(-surfToView, surfaceNormal) FS_PARAMS);
	vec3	prefilteredColor = SAMPLELOD_CUBE(EnvironmentMapSampler, sampleDir, roughness * kCubemapMipRange).rgb;
	vec2	envBRDF  = SAMPLE(BRDFLUTSampler, vec2(max(dot(surfaceNormal, surfToView), 0.0), roughness)).rg;
	vec3	specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	vec3	ambient = kD * diffuse + specular;
	
	return ambient;
}

#endif

void    FragmentMain(IN(SFragmentInput) fInput, OUT(SFragmentOutput) fOutput FS_ARGS)
{
	float   roughness = 1.0f;
	float   metalness = 1.0f;
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

#if defined(HAS_Lit)
	vec3 surfacePosition = fInput.fragWorldPosition;

	vec3 normalTex1 = SAMPLE(Lit_NormalMap, fInput.fragUV0).xyz;

	normalTex1 = 2.0f * normalTex1.xyz - vec3(1.0f, 1.0f, 1.0f);

	vec3	T = normalize(fInput.fragTangent.xyz);
	vec3	N = normalize(fInput.fragNormal.xyz);
	vec3	B = CROSS(N, T)  * fInput.fragTangent.w * GET_CONSTANT(SceneInfo, Handedness);
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
										color.rgb
										FS_PARAMS);

	vec3	ambientColor = GET_CONSTANT(LightInfo, AmbientColor);
	vec3	lightAmbient = computeAmbientBRDF(	surfToViewDir,
												surfaceNormal,
												surfaceRoughness,
												surfaceMetalness,
												color.rgb
												FS_PARAMS);

	color.rgb = lightDiffuse * lightColor + lightAmbient * ambientColor;
#endif
	fOutput.Output0 = color;
}
