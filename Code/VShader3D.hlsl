#pragma pack_matrix(row_major)
// an ultra simple hlsl vertex shader
#define MAX_SUBMESH_PER_DRAW 1024
struct _OBJ_ATTRIBUTES_
{
	float3 Kd;			// 12 bytes
	float d;			// 4 bytes
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
	int pixelShaderReturn;
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

OUTPUT_TO_RASTERIZER main(float3 inputVertex : POSITION, float3 uvw : UVW, float3 nrm : NRM) : SV_POSITION
{
	OUTPUT_TO_RASTERIZER output;
	float4 posInput = float4(inputVertex, 1);
	float4 posMain = mul(posInput, SceneData[0].matricies[mesh_ID]);
	float4 posWorld = posMain;
	float3 outNormal = mul(float4(nrm,0), SceneData[0].matricies[mesh_ID]).xyz;
	posMain = mul(posMain, SceneData[0].viewMatrix);
	posMain = mul(posMain, SceneData[0].projectionMatrix);
	output.posH = posMain;
	output.uvw = uvw;
	output.nrm = outNormal;
	output.posW = posWorld.xyz;
	return output;

};