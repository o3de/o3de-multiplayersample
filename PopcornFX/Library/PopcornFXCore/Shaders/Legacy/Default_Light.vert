
void    VertexMain(IN(SVertexInput) vInput, INOUT(SVertexOutput) vOutput VS_ARGS)
{
	// compute world space vertex position:

#if     HAS_SphereLight
	vec3    transformedPos = vInput.Position * (vInput.LightAttenuation_Range + vInput.SphereRadius) + vInput.LightPosition;
	vOutput.fragSphereRadius = vInput.SphereRadius;
#else
	vec3    transformedPos = vInput.Position * vInput.LightAttenuation_Range + vInput.LightPosition;
#endif

	mat4    viewProj = GET_CONSTANT(SceneInfo, ViewProj);
	vOutput.VertexPosition = mul(viewProj, vec4(transformedPos, 1.0));  // World to view
	vOutput.fragWorldPosition = vInput.Position;
	vOutput.fragViewProjPosition = vOutput.VertexPosition;
	vOutput.fragLightPosition = vInput.LightPosition;
	vOutput.fragLightAttenuation_Range = vInput.LightAttenuation_Range;
	vOutput.fragLight_Color = vInput.Light_Color;
}
