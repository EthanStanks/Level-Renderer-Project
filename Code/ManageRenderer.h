#pragma once
// Includes
#include <string>
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#include <chrono>




// Defines
#define MAX_SUBMESH_PER_DRAW 1024

// Global Variables
std::chrono::steady_clock::time_point elapsed;

GW::MATH::GMATRIXF mCamera;
GW::MATH::GMATRIXF mCameraCopy;


struct SavedLevel
{
	string pathtoMatrixData = "../";
	string pathtoH2BFolder = "../H2B/";
	string levelName;
};
struct Vector
{
	float x, y, z;
};
// Structs
struct Vertex
{
	float pos[3];
	float uvw[3];
	float nrm[3];
};
struct Attributes
{
	Vector Kd; float d;
	Vector Ks; float Ns;
	Vector Ka; float sharpness;
	Vector Tf; float Ni;
	Vector Ke; unsigned illum;
};
struct Batch
{
	unsigned indexCount, indexOffset;
};
struct Material
{
	Attributes attrib;
	/*const char* name;
	const char* map_Kd;
	const char* map_Ks;
	const char* map_Ka;
	const char* map_Ke;
	const char* map_Ns;
	const char* map_d;
	const char* disp;
	const char* decal;
	const char* bump;
	const void* padding[2];*/
};
struct Mesh
{
	const char* name;
	Batch drawInfo;
	unsigned materialIndex;
};
struct SHADER_MODEL_DATA
{
	// globally shared model data
	GW::MATH::GVECTORF sunDirection;
	GW::MATH::GVECTORF sunColor;
	GW::MATH::GVECTORF sunAmbient;
	GW::MATH::GVECTORF cameraPosition;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	// per sub-mesh transform and material data
	GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW]; // world space tranforms
	// TODO:: Remove array from matricies
	Material materials[MAX_SUBMESH_PER_DRAW]; // color/texture of surface
	int pixelShaderReturn;
};


class ManageRenderer
{
private:
	// Pipeline Stuff
	void CreateShaderStageInfo(VkPipelineShaderStageCreateInfo(&_stage_create_info)[2], VkShaderModule _moduleVertex, VkShaderModule _modulePixel)
	{
		// Create Stage Info for Vertex Shader
		_stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		_stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		_stage_create_info[0].module = _moduleVertex;
		_stage_create_info[0].pName = "main";
		// Create Stage Info for Fragment Shader
		_stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		_stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		_stage_create_info[1].module = _modulePixel;
		_stage_create_info[1].pName = "main";
	}
	void InitializeAssemblyInfo(VkPipelineInputAssemblyStateCreateInfo& _info)
	{
		_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		_info.primitiveRestartEnable = false;
	}
	void InitializeVertexBindingDesc(VkVertexInputBindingDescription& _vertex_binding_description)
	{
		_vertex_binding_description.binding = 0;
		_vertex_binding_description.stride = sizeof(Vertex);
		_vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}
	void InitializeVertexAttributeDescPosUvwNrm(VkVertexInputAttributeDescription(&vertex_attribute_description)[3])
	{
		vertex_attribute_description[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }; //pos
		vertex_attribute_description[1] = { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }; //uvw
		vertex_attribute_description[2] = { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6 }; //nrm
	}
	void InitializeInputVertexInfo(VkPipelineVertexInputStateCreateInfo& _input_vertex_info, VkVertexInputBindingDescription& _vertexBindingDesc, VkVertexInputAttributeDescription* _attributeDesc)
	{
		_input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		_input_vertex_info.vertexBindingDescriptionCount = 1;
		_input_vertex_info.pVertexBindingDescriptions = &_vertexBindingDesc;
		_input_vertex_info.vertexAttributeDescriptionCount = 3;
		_input_vertex_info.pVertexAttributeDescriptions = _attributeDesc;
	}
	void InitializeViewportInfo(VkPipelineViewportStateCreateInfo& _viewport_create_info, VkViewport& _viewport, VkRect2D& _scissor)
	{
		_viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		_viewport_create_info.viewportCount = 1;
		_viewport_create_info.pViewports = &_viewport;
		_viewport_create_info.scissorCount = 1;
		_viewport_create_info.pScissors = &_scissor;
	}
	void InitializeRasterStateInfo(VkPipelineRasterizationStateCreateInfo& _rasterization_create_info)
	{
		_rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		_rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
		_rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		_rasterization_create_info.lineWidth = 1.0f;
		_rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		_rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		_rasterization_create_info.depthClampEnable = VK_FALSE;
		_rasterization_create_info.depthBiasEnable = VK_FALSE;
		_rasterization_create_info.depthBiasClamp = 0.0f;
		_rasterization_create_info.depthBiasConstantFactor = 0.0f;
		_rasterization_create_info.depthBiasSlopeFactor = 0.0f;
	}
	void InitializeMultisamplingInfo(VkPipelineMultisampleStateCreateInfo& _multisample_create_info)
	{
		_multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		_multisample_create_info.sampleShadingEnable = VK_FALSE;
		_multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		_multisample_create_info.minSampleShading = 1.0f;
		_multisample_create_info.pSampleMask = VK_NULL_HANDLE;
		_multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		_multisample_create_info.alphaToOneEnable = VK_FALSE;
	}
	void InitializeDepthStencilInfo(VkPipelineDepthStencilStateCreateInfo& _depth_stencil_create_info)
	{
		_depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		_depth_stencil_create_info.depthTestEnable = VK_TRUE;
		_depth_stencil_create_info.depthWriteEnable = VK_TRUE;
		_depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		_depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
		_depth_stencil_create_info.minDepthBounds = 0.0f;
		_depth_stencil_create_info.maxDepthBounds = 1.0f;
		_depth_stencil_create_info.stencilTestEnable = VK_FALSE;
	}
	void InitializeColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& _color_blend_attachment_state)
	{
		_color_blend_attachment_state.colorWriteMask = 0xF;
		_color_blend_attachment_state.blendEnable = VK_FALSE;
		_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	void InitializeColorBlendInfo(VkPipelineColorBlendStateCreateInfo& _color_blend_create_info, VkPipelineColorBlendAttachmentState& _color_blend_attachment_state)
	{
		_color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		_color_blend_create_info.logicOpEnable = VK_FALSE;
		_color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
		_color_blend_create_info.attachmentCount = 1;
		_color_blend_create_info.pAttachments = &_color_blend_attachment_state;
		_color_blend_create_info.blendConstants[0] = 0.0f;
		_color_blend_create_info.blendConstants[1] = 0.0f;
		_color_blend_create_info.blendConstants[2] = 0.0f;
		_color_blend_create_info.blendConstants[3] = 0.0f;
	}
	void InitializeDynamicInfo(VkPipelineDynamicStateCreateInfo& _dynamic_create_info, VkDynamicState _dynamic_state[])
	{
		_dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		_dynamic_create_info.dynamicStateCount = 2;
		_dynamic_create_info.pDynamicStates = _dynamic_state;
	}
	void InitializeDescriptorLayoutBinding(VkDescriptorSetLayoutBinding& _descriptor_layout_binding)
	{
		_descriptor_layout_binding.binding = 0;
		_descriptor_layout_binding.descriptorCount = 1;
		_descriptor_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		_descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		_descriptor_layout_binding.pImmutableSamplers = nullptr;
	}
	void InitializeDescriptorInfo(VkDescriptorSetLayoutCreateInfo& _descriptor_create_info, VkDescriptorSetLayoutBinding& _descriptor_layout_binding)
	{
		_descriptor_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		_descriptor_create_info.flags = 0;
		_descriptor_create_info.bindingCount = 1;
		_descriptor_create_info.pBindings = &_descriptor_layout_binding;
		_descriptor_create_info.pNext = nullptr;
	}
	void InitializeDescriptorPoolInfo(VkDescriptorPoolCreateInfo& _descriptorpool_create_info, unsigned _max_frames)
	{
		_descriptorpool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		VkDescriptorPoolSize descriptorpool_size = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _max_frames };
		_descriptorpool_create_info.poolSizeCount = 1;
		_descriptorpool_create_info.pPoolSizes = &descriptorpool_size;
		_descriptorpool_create_info.maxSets = _max_frames;
		_descriptorpool_create_info.flags = 0;
		_descriptorpool_create_info.pNext = nullptr;
	}
	void InitializeDescriptorAllocationInfo(VkDescriptorSetAllocateInfo& _descriptorset_allocate_info, VkDescriptorSetLayout& _descriptorLayout, VkDescriptorPool _descriptorPool)
	{
		_descriptorset_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		_descriptorset_allocate_info.descriptorSetCount = 1;
		_descriptorset_allocate_info.pSetLayouts = &_descriptorLayout;
		_descriptorset_allocate_info.descriptorPool = _descriptorPool;
		_descriptorset_allocate_info.pNext = nullptr;
	}
	void InitializeWriteDescriptorSet(VkWriteDescriptorSet& _write_descriptorset)
	{
		_write_descriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		_write_descriptorset.descriptorCount = 1;
		_write_descriptorset.dstArrayElement = 0;
		_write_descriptorset.dstBinding = 0;
		_write_descriptorset.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	void InitializePipelineLayoutInfo(VkPipelineLayoutCreateInfo& _pipeline_layout_create_info, VkDescriptorSetLayout& _descriptorLayout, VkPushConstantRange& _pPushConstantRange)
	{
		_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		_pipeline_layout_create_info.setLayoutCount = 1;
		_pipeline_layout_create_info.pSetLayouts = &_descriptorLayout;
		_pipeline_layout_create_info.pushConstantRangeCount = 1;
		_pipeline_layout_create_info.pPushConstantRanges = &_pPushConstantRange;
	}
	void InitializePipelineInfo(VkGraphicsPipelineCreateInfo& _pipeline_create_info, VkPipelineShaderStageCreateInfo(&_stage_create_info)[2], VkPipelineInputAssemblyStateCreateInfo& _assembly_create_info, VkPipelineVertexInputStateCreateInfo& _input_vertex_info,
		VkPipelineViewportStateCreateInfo& _viewport_create_info, VkPipelineRasterizationStateCreateInfo& _rasterization_create_info, VkPipelineMultisampleStateCreateInfo& _multisample_create_info,
		VkPipelineDepthStencilStateCreateInfo& _depth_stencil_create_info, VkPipelineColorBlendStateCreateInfo& _color_blend_create_info, VkPipelineDynamicStateCreateInfo& _dynamic_create_info, VkPipelineLayout& _pipelineLayout,
		VkRenderPass& _renderPass)
	{
		_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		_pipeline_create_info.stageCount = 2;
		_pipeline_create_info.pStages = _stage_create_info;
		_pipeline_create_info.pInputAssemblyState = &_assembly_create_info;
		_pipeline_create_info.pVertexInputState = &_input_vertex_info;
		_pipeline_create_info.pViewportState = &_viewport_create_info;
		_pipeline_create_info.pRasterizationState = &_rasterization_create_info;
		_pipeline_create_info.pMultisampleState = &_multisample_create_info;
		_pipeline_create_info.pDepthStencilState = &_depth_stencil_create_info;
		_pipeline_create_info.pColorBlendState = &_color_blend_create_info;
		_pipeline_create_info.pDynamicState = &_dynamic_create_info;
		_pipeline_create_info.layout = _pipelineLayout;
		_pipeline_create_info.renderPass = _renderPass;
		_pipeline_create_info.subpass = 0;
		_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	}
public:

	// Load a shader file as a string of characters.
	std::string ShaderAsString(const char* shaderFilePath) {
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}

	void GetDimensions(GW::SYSTEM::GWindow _window, unsigned int& _w, unsigned int& _h)
	{
		_window.GetClientWidth(_w);
		_window.GetClientHeight(_h);
	}

	float VectorMagnitude(float x, float y, float z) // the length of the vector
	{
		return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	}

	GW::MATH::GVECTORF VectorNormalize(float x, float y, float z)
	{

		float magnitude = VectorMagnitude(x, y, z);
		GW::MATH::GVECTORF vec = { x / magnitude , y / magnitude, z / magnitude };
		return vec;
	}

	void GetInputForPixelShader(GW::INPUT::GInput _input, int& _returnValue)
	{
		float val1 = 0;
		float val2 = 0;
		float val3 = 0;
		float val4 = 0;
		float val5 = 0;
		float val6 = 0;
		_input.GetState(G_KEY_F1, val1);
		_input.GetState(G_KEY_F2, val2);
		_input.GetState(G_KEY_F3, val3);
		_input.GetState(G_KEY_F4, val4);
		_input.GetState(G_KEY_F5, val5);
		_input.GetState(G_KEY_F6, val6);

		if (val1 > 0)
			_returnValue = 1;
		else if (val2 > 0)_returnValue = 2;
		else if (val3 > 0) _returnValue = 3;
		else if (val4 > 0) _returnValue = 4;
		else if (val5 > 0) _returnValue = 5;
		else if (val6 > 0) _returnValue = 6;
	}
	void RotateWorldMatrixSlowlyOverTime(GW::MATH::GMatrix _math, GW::MATH::GMATRIXF& _wm)
	{
		auto currentTime = std::chrono::steady_clock::now();
		float secondsPassed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - elapsed).count() / 1000000.0f;
		elapsed = currentTime;
		float fDeltaTime = secondsPassed;
		_math.RotateYLocalF(_wm, fDeltaTime, _wm); // rotate world matrix slowly over time
	}

	void PipelineDynamicSettings(VkCommandBuffer& _commandBuffer, VkPipeline& _pipeline, VkViewport& _viewport, VkRect2D& _scissor, VkPipelineBindPoint _bindPoint)
	{
		vkCmdSetViewport(_commandBuffer, 0, 1, &_viewport);
		vkCmdSetScissor(_commandBuffer, 0, 1, &_scissor);
		vkCmdBindPipeline(_commandBuffer, _bindPoint, _pipeline);
	}
	void BindVertexInputDescripterSets(VkCommandBuffer& _commandBuffer, VkDevice& _device, VkBuffer& _vertexHandle, VkBuffer& _indexBuffer,
		std::vector<VkDeviceMemory>& _storageData, unsigned int& _currentBuffer, std::vector<VkDescriptorSet>& _descriptorSet, VkPipelineLayout& _pipelineLayout,
		SHADER_MODEL_DATA& _shaderData)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_vertexHandle, offsets);
		vkCmdBindIndexBuffer(_commandBuffer, _indexBuffer, offsets[0], VK_INDEX_TYPE_UINT32);
		GvkHelper::write_to_buffer(_device, _storageData[_currentBuffer], &_shaderData, sizeof(SHADER_MODEL_DATA));
		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			_pipelineLayout, 0, 1, &_descriptorSet[_currentBuffer], 0, nullptr);
	}
	void DrawMesh(VkCommandBuffer& _commandBuffer, VkPipelineLayout& _pipelineLayout, VkShaderStageFlags _stageFlags, UINT _push,
		unsigned int _indexCount, unsigned int _indexOffset)
	{
		// tell what material to use
		vkCmdPushConstants(_commandBuffer, _pipelineLayout, _stageFlags, 0, sizeof(_push), &_push);
		vkCmdDrawIndexed(_commandBuffer, _indexCount, 1, _indexOffset, 0, 0);
	}

	void InitializePipeline(GW::GRAPHICS::GVulkanSurface& _vlk, VkShaderModule &_vertexShader, VkShaderModule &_pixelShader, unsigned int _width, unsigned int _height,
		VkDevice &_device, VkDescriptorSetLayout &_descriptorLayout, VkDescriptorPool &_descriptorPool, std::vector<VkDescriptorSet> &_descriptorSet, std::vector<VkBuffer> &_storageHandle, unsigned _max_frames,
		VkPipelineLayout &_pipelineLayout, VkPipeline &_pipeline)
	{
		// Create Pipeline & Layout (Thanks Tiny!)
		VkRenderPass renderPass;
		_vlk.GetRenderPass((void**) &renderPass);

		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		CreateShaderStageInfo(stage_create_info, _vertexShader, _pixelShader);

		// Assembly State
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
		InitializeAssemblyInfo(assembly_create_info);

		// Vertex Input State
		VkVertexInputBindingDescription vertex_binding_description = {};
		InitializeVertexBindingDesc(vertex_binding_description);

		VkVertexInputAttributeDescription vertex_attribute_description[3];
		InitializeVertexAttributeDescPosUvwNrm(vertex_attribute_description);

		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		InitializeInputVertexInfo(input_vertex_info, vertex_binding_description, vertex_attribute_description);

		// Viewport State (we still need to set this up even though we will overwrite the values)
		VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
		VkRect2D scissor = { {0, 0}, {_width, _height} };

		VkPipelineViewportStateCreateInfo viewport_create_info = {};
		InitializeViewportInfo(viewport_create_info, viewport, scissor);

		// Rasterizer State
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
		InitializeRasterStateInfo(rasterization_create_info);

		// Multisampling State
		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		InitializeMultisamplingInfo(multisample_create_info);

		// Depth-Stencil State
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
		InitializeDepthStencilInfo(depth_stencil_create_info);

		// Color Blending Attachment & State
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
		InitializeColorBlendAttachmentState(color_blend_attachment_state);
		InitializeColorBlendInfo(color_blend_create_info, color_blend_attachment_state);

		// Dynamic State 
		VkDynamicState dynamic_state[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }; // By setting these we do not need to re-create the pipeline on Resize
		VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
		InitializeDynamicInfo(dynamic_create_info, dynamic_state);

		VkDescriptorSetLayoutCreateInfo descriptor_create_info = {};
		VkDescriptorSetLayoutBinding descriptor_layout_binding = {};
		InitializeDescriptorLayoutBinding(descriptor_layout_binding);
		InitializeDescriptorInfo(descriptor_create_info, descriptor_layout_binding);

		// Descriptor layout
		VkResult r = vkCreateDescriptorSetLayout(_device, &descriptor_create_info, nullptr, &_descriptorLayout);

		VkDescriptorPoolCreateInfo descriptorpool_create_info = {};
		InitializeDescriptorPoolInfo(descriptorpool_create_info, _max_frames);
		vkCreateDescriptorPool(_device, &descriptorpool_create_info, nullptr, &_descriptorPool);

		VkDescriptorSetAllocateInfo descriptorset_allocate_info = {};
		InitializeDescriptorAllocationInfo(descriptorset_allocate_info, _descriptorLayout, _descriptorPool);
		_descriptorSet.resize(_max_frames);
		for (int i = 0; i < _max_frames; ++i) {
			vkAllocateDescriptorSets(_device, &descriptorset_allocate_info, &_descriptorSet[i]);
		}

		VkWriteDescriptorSet write_descriptorset = {};
		InitializeWriteDescriptorSet(write_descriptorset);
		for (int i = 0; i < _max_frames; ++i) {
			write_descriptorset.dstSet = _descriptorSet[i];
			VkDescriptorBufferInfo dbinfo = { _storageHandle[i], 0, VK_WHOLE_SIZE };
			write_descriptorset.pBufferInfo = &dbinfo;
			vkUpdateDescriptorSets(_device, 1, &write_descriptorset, 0, nullptr);
		}

		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		VkPushConstantRange pPushConstantRange = { stageFlags, 0, sizeof(unsigned) };
		InitializePipelineLayoutInfo(pipeline_layout_create_info, _descriptorLayout, pPushConstantRange);
		vkCreatePipelineLayout(_device, &pipeline_layout_create_info,
			nullptr, &_pipelineLayout);

		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		InitializePipelineInfo(pipeline_create_info, stage_create_info, assembly_create_info, input_vertex_info, viewport_create_info, rasterization_create_info, multisample_create_info,
			depth_stencil_create_info, color_blend_create_info, dynamic_create_info, _pipelineLayout, renderPass);

		vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1,
			&pipeline_create_info, nullptr, &_pipeline);
	}
	void InitializeShaders(VkDevice &_device, VkShaderModule &_vertexShader, VkShaderModule &_pixelShader, std::string &_VShader3D, std::string &_VPixel3D)
	{
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, false);
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		// Create Vertex Shader
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, _VShader3D.c_str(), _VShader3D.length(),
			shaderc_vertex_shader, "main.vert", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(_device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &_vertexShader);
		shaderc_result_release(result); // done
		// Create Pixel Shader
		result = shaderc_compile_into_spv( // compile
			compiler, _VPixel3D.c_str(), _VPixel3D.length(),
			shaderc_fragment_shader, "main.frag", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(_device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &_pixelShader);
		shaderc_result_release(result); // done
		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}
	void H2BBatchToBatch(Batch& batch, H2B::BATCH hbatch)
	{
		batch.indexCount = hbatch.indexCount;
		batch.indexOffset = hbatch.indexOffset;
	}
	void H2BVectorToVector(Vector& gvec, H2B::VECTOR hvec)
	{
		gvec.x = hvec.x;
		gvec.y = hvec.y;
		gvec.z = hvec.z;
	}
	GW::MATH::GMATRIXF LoaderMatrixToGMATRIXF(Matrix m)
	{
		GW::MATH::GMATRIXF matrix;
		matrix.row1.x = m.row1[0];
		matrix.row1.y = m.row1[1];
		matrix.row1.z = m.row1[2];
		matrix.row1.w = m.row1[3];

		matrix.row2.x = m.row2[0];
		matrix.row2.y = m.row2[1];
		matrix.row2.z = m.row2[2];
		matrix.row2.w = m.row2[3];

		matrix.row3.x = m.row3[0];
		matrix.row3.y = m.row3[1];
		matrix.row3.z = m.row3[2];
		matrix.row3.w = m.row3[3];

		matrix.row4.x = m.row4[0];
		matrix.row4.y = m.row4[1];
		matrix.row4.z = m.row4[2];
		matrix.row4.w = m.row4[3];
		return matrix;
	}

};