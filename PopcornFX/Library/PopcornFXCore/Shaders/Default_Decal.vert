
void    VertexMain(IN(SVertexInput) vInput, INOUT(SVertexOutput) vOutput VS_ARGS)
{
	vec3 	transformedPos = mul(vInput.DecalTransform, vec4(vInput.Position, 1.0)).xyz;
	mat4    viewProj = GET_CONSTANT(SceneInfo, ViewProj); 

	vOutput.VertexPosition = mul(viewProj, vec4(transformedPos, 1.0));  // World to view
	vOutput.fragWorldPosition = vInput.Position;
	vOutput.fragViewProjPosition = vOutput.VertexPosition;
	vOutput.fragInverseDecalTransform = vInput.InverseDecalTransform;
}
