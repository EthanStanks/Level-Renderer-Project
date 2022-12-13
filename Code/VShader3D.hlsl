#pragma pack_matrix(row_major)
// an ultra simple hlsl vertex shader
#define MAX_SUBMESH_PER_DRAW 1024
struct Attributes
{
	float3 Kd; float d;
	float3 Ks; float Ns;
	float3 Ka; float sharpness;
	float3 Tf; float Ni;
	float3 Ke; uint illum;
};
struct Material
{
	Attributes attrib;
	/*char name;
	char map_Kd;
	char map_Ks;
	char map_Ka;
	char map_Ke;
	char map_Ns;
	char map_d;
	char disp;
	char decal;
	char bump;
	char padding[2];*/
};
struct SHADER_MODEL_DATA
{
	float4 sunDirection;								// 16 bytes
	float4 sunColor;									// 16 bytes
	float4 sunAmbient;
	float4 cameraPosition;
	float4x4 viewMatrix;								// 64 bytes
	float4x4 projectionMatrix;							// 64 bytes
	float4x4 matricies[MAX_SUBMESH_PER_DRAW];			// 64 bytes
	Material materials[MAX_SUBMESH_PER_DRAW];	// 80 bytes
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
	float4 posMain = mul(posInput, SceneData[0].matricies[0]);
	float4 posWorld = posMain;
	float3 outNormal = mul(float4(nrm,0), SceneData[0].matricies[0]).xyz;
	posMain = mul(posMain, SceneData[0].viewMatrix);
	posMain = mul(posMain, SceneData[0].projectionMatrix);
	output.posH = posMain;
	output.uvw = uvw;
	output.nrm = outNormal;
	output.posW = posWorld.xyz;
	return output;

};