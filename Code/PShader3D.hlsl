#define MAX_SUBMESH_PER_DRAW 1024
struct _OBJ_ATTRIBUTES_
{
	float3 Kd;			// 12 bytes
	float d; 			// 4 bytes
	float3 Ks;			// 12 bytes
	float Ns;			// 4 bytes
	float3 Ka;			// 12 bytes
	float sharpness;	// 4 bytes
	float3 Tf;			// 12 bytes
	float Ni;			// 4 bytes
	float3 Ke;			// 12 bytes
	uint illum;			// 4 bytes
};
struct SHADER_MODEL_DATA
{
	float4 sunDirection;								// 16 bytes
	float4 sunColor;									// 16 bytes
	float3 sunAmbient;
	float4 cameraPosition;
	float4x4 viewMatrix;								// 64 bytes
	float4x4 projectionMatrix;							// 64 bytes
	float4x4 matricies[MAX_SUBMESH_PER_DRAW];			// 64 bytes
	_OBJ_ATTRIBUTES_ materials[MAX_SUBMESH_PER_DRAW];	// 80 bytes
	uint pixelShaderReturn;
};
StructuredBuffer<SHADER_MODEL_DATA> SceneData;
[[vk::push_constant]]
cbuffer MESH_INDEX
{
	uint mesh_ID;
};
struct OUTPUT_TO_RASTERIZER
{
	float4 posH : SV_POSITION; // homogeneous projection space
	float3 uvw : UVW; // UV coordinate with w
	float3 nrm : NORMAL; // normal in world space (for lighting)
	float3 posW : WORLD; // position in world space (for lighting)
};
float4 main(OUTPUT_TO_RASTERIZER output) : SV_TARGET
{
	uint pixelShaderReturn = SceneData[0].pixelShaderReturn;
	float3 nrm = normalize(output.nrm);
	float lightIntensity = 1;
	float3 lightDirecion = float3(SceneData[0].sunDirection.xyz * -1);
	float3 lightEmmision = SceneData[0].materials[mesh_ID].Ke;
	float3 materialColor = SceneData[0].materials[mesh_ID].Kd;
	float specularExponent = SceneData[0].materials[mesh_ID].Ns;
	float3 specularReflectivity = SceneData[0].materials[mesh_ID].Ks;
	float3 cameraPosition = SceneData[0].cameraPosition.xyz;
	float3 sunColor = SceneData[0].sunColor;
	float3 worldPosition = output.posW;
	float3 ambiantLight = SceneData[0].sunAmbient;

	float lightRatio = saturate(dot(lightDirecion, nrm));
	float3 directionalLight = /*materialColor * */sunColor * lightRatio;

	float3 indirectionalLight = materialColor * ambiantLight;

	float3 specularViewDirection = normalize(cameraPosition - worldPosition);
	float3 specularHalfVector = normalize(lightDirecion + specularViewDirection);
	float specularLightIntensity = pow(max(dot(nrm, specularHalfVector), 0), specularExponent);
	float3 specularLight = sunColor * specularLightIntensity * specularReflectivity;

	float3 lightColor = saturate(directionalLight + indirectionalLight) * materialColor + specularLight + lightEmmision;

	// Returns

	if (pixelShaderReturn == 1) // Directional Light
	{
		return float4(directionalLight, 1);
	}
	else if (pixelShaderReturn == 2) // Indirectional Light
	{
		return float4(indirectionalLight, 1);
	}
	else if (pixelShaderReturn == 3) // Directional Light w/ Indirectional Light
	{
		return float4(saturate(directionalLight + indirectionalLight), 1);
	}
	else if (pixelShaderReturn == 4) // Specular Light
	{
		return float4(specularLight, 1);
	}
	else if (pixelShaderReturn == 5) // Directional, Indirectional, and Reflected Light
	{
		return float4(lightColor, 1);
	}
	else if (pixelShaderReturn == 6) // Material Color with No Light
	{
		return float4(materialColor, 1);
	}

};